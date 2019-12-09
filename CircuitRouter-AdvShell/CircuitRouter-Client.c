#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "CircuitRouter-Client.h"
#include "ServerUtil.h"

#define INPTSIZE 100
#define PIDSIZE  5
#define BUFSIZE INPTSIZE+PIDSIZE
#define RESPSIZE 500
#define PROMPT "[CrctRtr-Clt]$ "

char* servInName;  // sendig to server pipe
char* servErrName; // receiving err from server pipe
char* servOutName; // receiving out from server pipe

/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName) {
    printf("Usage: %s <server pipe directory>\n", appName);
    exit(1);
}

/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]) {
    long i;
    opterr = 0;

	if(argc < 2) {
		printf("You must specify a filename.\n");
		opterr++;
	}

    for (i = optind; i < argc-1; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

	servInName = argv[optind]; // connection pipename is last argument given

    if (opterr)
        displayUsage(argv[0]);
}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
	int status = 1, pidlgth, retVal, maxfd, nBytes;
	int servInFd, servOutFd, servErrFd;
	fd_set srvResponse;
	char* inputBfr; // points to introcuced command
	char* request;  // points to entire request
	char* response; // points to server response

	parseArgs(argc, argv);
	if(access(servInName, F_OK) == -1) fatal("Connection error");

	// set input buffer
	if(!(request = (char*)malloc(sizeof(char) * BUFSIZE))) fatal("Allocation error");
	memset(request, '\0', sizeof(char) * BUFSIZE);
	if(sprintf(request, "%d ", getpid()) < 0) fatal("Initializing request");
	pidlgth = strlen(request);
	inputBfr = &(request[pidlgth]);

	// set reponse buffer
	response = (char*)malloc(sizeof(char)* RESPSIZE);
	if(!response) fatal("Allocation error");
	memset(response, 0, sizeof(char)* RESPSIZE);

	// set response pipes
	servOutName = getServerOutName(servInName, (int) getpid());
	servErrName = getServerErrName(servInName, (int) getpid());
	if(!(servOutName && servErrName)) fatal("Creating pipe names");

	if(unlink(servOutName) == -1 && errno != ENOENT) fatal("Unlinking pipes");
	if(unlink(servErrName) == -1 && errno != ENOENT) fatal("Unlinking pipes");
	if(mkfifo(servOutName, 0777) < 0) fatal("Creating out pipe");
	if(mkfifo(servErrName, 0777) < 0) fatal("Creating err pipe");


	// connect to server
	if((servInFd = open(servInName, O_WRONLY)) < 0) fatal("Connection error");

	while(status){ // eventually status will change

		prompt( CYN PROMPT RESET, inputBfr, BUFSIZE - pidlgth);

		if(!strcmp(inputBfr, "exit")) {
			break;
		}
		if(	write(servInFd, request, BUFSIZE) == -1) fatal("Communication error");

		// wait for server response
		// important: open in same order to avoid pipe locks (O_NONBLOCK not necessary)
		if((servOutFd = open(servOutName, O_RDONLY)) < 0) fatal("Connection error");
		if((servErrFd = open(servErrName, O_RDONLY )) < 0) fatal("Connection error");
		maxfd = servErrFd + 1;

        FD_ZERO(&srvResponse);
        FD_SET(servOutFd, &srvResponse);
        FD_SET(servErrFd, &srvResponse);

        if((retVal = select(maxfd, &srvResponse, NULL, NULL, NULL)) > 0) {
			// handle out and err pipes differently
			if(FD_ISSET(servOutFd, &srvResponse)) {
				while((nBytes = read(servOutFd, response, RESPSIZE)) != 0) {
					if(nBytes == 0) fatal("OutPipe error");
					handleErr(response, sizeof(char) * RESPSIZE);
				}
			}
            if(FD_ISSET(servErrFd, &srvResponse)) {
				while((nBytes = read(servErrFd, response, RESPSIZE)) != 0) {
					if(nBytes == 0) fatal("ErrPipe error");
					handleErr(response, sizeof(char) * RESPSIZE);
				}
			}
        }

		// close response pipes
		if(	close(servOutFd) == -1) fatal("Error closing pipe");
		if(	close(servErrFd) == -1) fatal("Error closing pipe");
	}

	if(	close(servInFd) == -1) fatal("Error closing pipe");

	if(unlink(servOutName) == -1) fatal("Unlinking response pipe");
	if(unlink(servErrName) == -1) fatal("Unlinking response pipe");
	
	free(servOutName);
	free(servErrName);
	free(request);
	free(response);
	return 0;
}


// handleOut and handleErr are pretty similar in this implementation
// keeping them apart to add flexibility

/* =============================================================================
 * handleOut
 *  handle server normal responses
 * =============================================================================
 */
void handleOut(char* response, int size) {
	printf("%s", response);
	memset(response, '\0', size);
}

/* =============================================================================
 * handleErr
 *  handle server error responses
 * =============================================================================
 */
void handleErr(char* response, int size) {
	// maybe add some colors?
	printf("%s", response);
	memset(response, '\0', size);
}
