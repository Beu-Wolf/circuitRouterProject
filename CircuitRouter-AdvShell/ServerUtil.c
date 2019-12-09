#include "ServerUtil.h"


/* =============================================================================
 * fatal
 *  Prints message and exit with error 
 * =============================================================================
 */
void fatal(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


/* =============================================================================
 * getServerInpName
 * =============================================================================
 */
char* getServerInpName() {
    // further change servername protocol
    return PIPENAME;
}


// getServerOutName and getServerErrName are pretty similar in this implementation
// keeping them apart to add flexibility

/* =============================================================================
 * getServerOutName
 *  returns output pipe based on client PID
 * =============================================================================
 */
char* getServerOutName(const char* servNm, int identifier) {
    char* cliOutName = malloc(sizeof(char) * (strlen(servNm) + EXTRALEN));
    if(!cliOutName) return NULL;
    
    if(sprintf(cliOutName, "%s%d%s", servNm, identifier, OUT) < 0) return NULL;
    return cliOutName;
}

/* =============================================================================
 * getServerErrName
 *  returns error pipe based on client PID
 * =============================================================================
 */
char* getServerErrName(const char* servNm, int identifier) {
    char* cliErrName = malloc(sizeof(char) * (strlen(servNm) + EXTRALEN));
    if(!cliErrName) return NULL;
    
    if(sprintf(cliErrName, "%s%d%s", servNm, identifier, ERR) < 0) return NULL;
    return cliErrName;
}


/* =============================================================================
 * prompt
 *  copies written command in stdin to buffer
 * =============================================================================
 */
void prompt(char* promptMsg, char* buffer, unsigned long size) {
	int i = 0;
	char c, hasChar = 0;
	
    if(promptMsg != NULL) {
        printf("%s", promptMsg);
    }

	// read until new line or exceed size or EOF or '\0'
	while(i < size-1 && (c = getc(stdin)) != '\n' && c != '\0' && c != EOF) {
		buffer[i++] = c;
		if(!hasChar) hasChar = !isspace(c);
	}

	// buffer overflow protection
	if(i >= size-1) {
		printf("Input too long: truncating\n");
		while(getchar() != '\n'); //clean input buffer
	}
	
	if(!hasChar) i = 0; //ignore whitespace lines

	// keep msg buffer clean
	while(i < size) buffer[i++] = '\0'; 
}

/* =============================================================================
 * interpretCommand
 *  Interpret and validate command arguments
 * =============================================================================
 */
int interpretCommand(int nargs, char** args) {
    if(nargs > 0){
        if(!strcmp(args[0], "run")) {
            return RUN;
        } else if(!strcmp(args[0], "exit")) {
            return EXIT;
        } else {
            return UNSUPPORTED;
        }
    }
    return EMPTY;
}
