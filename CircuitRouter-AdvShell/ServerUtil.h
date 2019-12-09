#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define PIPENAME "/tmp/CircuitRouter-AdvShell.pipe"
#define EXTRALEN 10
#define OUT "out"
#define ERR "err"

#define UNSUPPORTED -2
#define INVALID -1 // for known commands with invalid format
#define EXIT 0
#define EMPTY 1
#define RUN 2

// server colors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


/* =============================================================================
 * fatal
 *  Prints message and exit with error 
 * =============================================================================
 */
void fatal(char* msg);

/* =============================================================================
 * getServerInpName
 * =============================================================================
 */
char* getServerInpName();

/* =============================================================================
 * getServerOutName
 *  returns output pipe based on client PID
 * =============================================================================
 */
char* getServerOutName(const char* servNm, int identifier);

/* =============================================================================
 * getServerErrName
 *  returns error pipe based on client PID
 * =============================================================================
 */
char* getServerErrName(const char* servNm, int pid);

/* =============================================================================
 * prompt
 *  copies written command in stdin to buffer
 * =============================================================================
 */
void prompt(char* promptMsg, char* buffer, unsigned long size);

/* =============================================================================
 * interpretCommand
 *  Interpret and validate command arguments
 * =============================================================================
 */
int interpretCommand(int nargs, char** args);
