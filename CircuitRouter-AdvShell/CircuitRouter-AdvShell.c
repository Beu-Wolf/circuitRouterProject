#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../lib/commandlinereader.h"
#include "CircuitRouter-AdvShell.h"
#include "ServerUtil.h"

char PRINT_CLIENT_INPUT = FALSE;

vector_t* children;
const char* servName;
int maxChildren, availableChildren;

sigset_t ss; // signal mask


/* =============================================================================
 * handleSIGCHLD
 * =============================================================================
 */
void handleSIGCHLD(int s) {
    int status;
    pid_t pid;
    TIMER_T endTime;
    long size;
    process_t* finished;
    
    TIMER_READ(endTime);
    while((pid = waitpid(-1, &status, WNOHANG))) {
        // wait for every finished children
        if(pid == -1) {
            if(errno == ECHILD) break;
            fatal("wait error");
        }
    
        // terminated child definetly exists in children
        size = vector_getSize(children);
        for(long i = 0; i < size; i++) { //search child
            finished = (process_t*)vector_at(children, i);
            if(finished->PID == pid) break;
        }
        finished->end = endTime;
        finished->wstatus = status;

        availableChildren++;
    }
}


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                            (defaults)\n");
    printf("    s          [s]how requests      (false)\n");
    printf("    m          [m]ax children       (unlimited)\n");
    printf("    h          [h]elp message       (false)\n");
    exit(1);
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]){
    long i;
    long opt;

    opterr = 0;

    maxChildren = availableChildren = 0;

    while ((opt = getopt(argc, argv, "h?sm:")) != -1) {
        switch (opt) {
            case 's':
                PRINT_CLIENT_INPUT = TRUE;
                break;
            case 'm':
                availableChildren = maxChildren = atoi(optarg);
                break;
            case '?':
            case 'h':
            default:
                opterr++;
                break;
        }
    }

    for (i = optind; i < argc-1; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if (opterr) {
        displayUsage(argv[0]);
    }
}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv) {
    struct sigaction newAction, oldAction;
    fd_set readfds;
    int fserv, maxfd, retVal;
    char* command;
    char status = 1;

    
    parseArgs(argc, (char** const)argv);

    servName = getServerInpName();

    command = malloc(CMDLEN * sizeof(char));
    if(command == NULL) fatal("Allocation error");
    
    children = vector_alloc(0);

    // opening server
    if(unlink(servName) == -1 && errno != ENOENT) fatal("Unlinking pipe");
    if(mkfifo(servName, 0777) < 0) fatal("Creating pipe");
    printf("Listening to:%s\n", servName);

    // set up signal listeners and mask
    bzero(&newAction, sizeof(struct sigaction));
    newAction.sa_handler = handleSIGCHLD;
    if(sigemptyset (&newAction.sa_mask) == -1) fatal("sigemptyset error");
    newAction.sa_flags = SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &newAction, &oldAction) == -1) fatal("error changing signal handling");
    
    if(sigemptyset(&ss) == -1) fatal("Synchronizing child");
    if(sigaddset(&ss, SIGCHLD) == -1) fatal("Synchronizing child");


    // wait for connection
    if((fserv = open(servName, O_RDWR | O_NONBLOCK)) < 0) fatal("Opening pipe");
    maxfd = fserv + 1;

    while(status) {

        memset(command, '\0', CMDLEN);

        // listen for input
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(fserv, &readfds);

        if((retVal = select(maxfd, &readfds, NULL, NULL, NULL)) > 0) {
            // prioritizing server requests
            if(FD_ISSET(0, &readfds))
                status = handleServer(command);   

            if(FD_ISSET(fserv, &readfds))
                handleClient(command, fserv);
        } else if(retVal < 0 && errno != EINTR)
            fatal("While waiting for command");

    }

    // close session
    if(close(fserv) == -1) fatal("error closing server pipe");

    // default signal handling
    if(sigaction(SIGCHLD, &oldAction, NULL) == -1) fatal("error changing signal handling");

    // print children status
    printStatus();

    free(command);
    vector_free(children);
    if(unlink(servName) == -1) fatal("Unlink error");
    return 0;
}


/* =============================================================================
 * handleServer
 *  handle server commands
 * =============================================================================
 */
int handleServer(char* command) {
    pid_t pid;
    int nargs, cmdCode;
    char** args = NULL;

    if(!(args = (char**)malloc(sizeof(char*) * NARGS))) fatal("allocating memory");
    memset(args, 0, NARGS);

    nargs = readLineArguments(0, args, NARGS, command, CMDLEN);

    cmdCode = interpretCommand(nargs, args);
    switch(cmdCode) {
        case INVALID:
            printf("%s", INVALID_MSG);
            break;
        case UNSUPPORTED:
            printf("%s", UNSUPPORTED_MSG);
            break;
        case RUN:
            while(maxChildren && !availableChildren) // wait for available children
                pause(); // use conditional var?
            
            // ensure availableChildren integrity
            if(sigprocmask(SIG_BLOCK, &ss, NULL) == -1) fatal("Synchronizing child");
            availableChildren--;

            TIMER_T startTime;
            TIMER_READ(startTime);

            pid = fork();
            if(pid < 0) {
                fatal("Forking error");
            } else if(pid == 0) { // child process
                execv("./../CircuitRouter-ParSolver/CircuitRouter-ParSolver", args);
                fatal("execv error");
            } else { // parent process
                registerChild(children, pid, startTime);
            }
            
            // avoid child exiting before being registered
            if(sigprocmask(SIG_UNBLOCK, &ss, NULL) == -1) fatal("Synchronizing child");
            
            break;
        case EXIT:
        case EMPTY:
            break;
        default:
            printf("Invalid command\n");
            cmdCode = UNSUPPORTED;
            break;
    }

    free(args);
    return cmdCode;
}


/* =============================================================================
 * handleClient
 *  handle client requests
 * =============================================================================
 */
int handleClient(char* command, int fserv) {
    int nargs, clOut, clErr, cmdCode = 1;
    char** input = NULL;
    char** args;
    char* sendPipeOut;
    char* sendPipeErr;
    pid_t pid;

    input = (char**)malloc(sizeof(char*) * NARGS);

    if(input == NULL)
        fatal("Allocation error");
    
    memset(input, 0 ,NARGS);

    nargs = readLineArguments(fserv, input, NARGS, command, CMDLEN);
    if(nargs <= 0) return -1;
    args = &(input[1]);
    if(PRINT_CLIENT_INPUT) logClientInput(nargs, input);

    // open client response pipes
    sendPipeOut = getServerOutName(servName, atoi(input[0]));
    sendPipeErr = getServerErrName(servName, atoi(input[0]));

    clOut = -1;
    while(clOut < 0) // retry to open if failed due to interruption
        if((clOut = open(sendPipeOut, O_WRONLY )) < 0 && errno != EINTR)
            fatal("Out pipe connection error");

    clErr = -1;
    while(clErr < 0) // retry to open if failed due to interruption
        if((clErr = open(sendPipeErr, O_WRONLY )) < 0 && errno != EINTR)
            fatal("Err pipe connection error");


    cmdCode = interpretCommand(nargs-1, args); // nargs - 1 to ignore PID
    switch(cmdCode) {
        case INVALID:
            write(clErr, INVALID_MSG, strlen(INVALID_MSG));
            break;
        case UNSUPPORTED:
            write(clErr, UNSUPPORTED_MSG, strlen(UNSUPPORTED_MSG));
            break;
        case RUN:
            while(maxChildren && !availableChildren)
                pause();

            // ensure availableChildren integrity
            if(sigprocmask(SIG_BLOCK, &ss, NULL) == -1) fatal("Synchronizing child");
            availableChildren--;

            TIMER_T startTime;
            TIMER_READ(startTime);

            pid = fork();
            if(pid < 0) {
                fatal("Error while forking");
            } else if(pid == 0) {
                // redirect stdout and stderr
                if(close(1) == -1) fatal("closing error");
                if(dup(clOut) == -1) fatal("duplication error");
                if(close(2) == -1) fatal("closing error");
                if(dup(clErr) == -1) fatal("duplication error");
                if(close(clOut) == -1) fatal("closing error");
                if(close(clErr) == -1) fatal("closing error");
                execv("./../CircuitRouter-ParSolver/CircuitRouter-ParSolver", args);
                fatal("execv error");
            } else { // parent process
                registerChild(children, pid, startTime);
            }
            
            // avoid child exiting before being registered
            if(sigprocmask(SIG_UNBLOCK, &ss, NULL) == -1) fatal("Synchronizing child");
        
            break;
        case EXIT:
        case EMPTY:
            break;
        default:
            write(clErr, UNSUPPORTED_MSG, strlen(UNSUPPORTED_MSG));
            return UNSUPPORTED;
    }
    if(close(clOut) == -1) fatal("closing error");
    if(close(clErr) == -1) fatal("closing error");
    free(input);
    free(sendPipeOut);
    free(sendPipeErr);
    return cmdCode;
}


/* =============================================================================
 * registerChild
 *  save new child on saved child list
 * =============================================================================
 */
void registerChild(vector_t* v, pid_t pid, TIMER_T startTime) {
    process_t* newRun = malloc(sizeof(process_t));
    if(newRun == NULL)
        fatal("allocating memory");
    
    newRun->PID = pid;
    newRun->start = startTime;
    newRun->end = startTime; // mark endTime (updated when SIGCHLD comes up)
    vector_pushBack(v, (void*)newRun);
}


/* =============================================================================
 * printStatus
 *  display launched processes information
 * =============================================================================
 */
void printStatus() {
    long size;
    size = vector_getSize(children);
    process_t* current;

    for(long i = 0; i < size; i++) {
        current = vector_at(children, i);
        if(!TIMER_DIFF_SECONDS(current->start, current->end)) {
            if(waitpid(current->PID, &(current->wstatus), 0) == -1) fatal("wainting for child error");
            TIMER_READ(current->end);
        }

        printf("CHILD EXITED (PID=%d; return %s; %d s)\n",
            current->PID,
            (WIFEXITED(current->wstatus) && !WEXITSTATUS(current->wstatus) && !WIFSIGNALED(current->wstatus)) ? "OK" : "NOK",
            (int)TIMER_DIFF_SECONDS(current->start, current->end) / 1
        );

        free(current);
    }
}


/* =============================================================================
 * logClientInput
 *  display client requests (shows client identifier)
 * =============================================================================
 */
void logClientInput(int nargs, char** input) {
    printf( CYN "[%s]:", input[0]);
    for(int i = 1; i < nargs; i++) {
        printf(" %s", input[i]);
    }
    printf( RESET "\n");
}