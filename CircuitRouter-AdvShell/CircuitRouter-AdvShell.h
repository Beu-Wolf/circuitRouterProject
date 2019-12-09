#include <sys/types.h>
#include "lib/timer.h"
#include  "lib/vector.h"

#define NARGS 10
#define BUFSIZE 400
#define PATHSIZE 100
#define CMDLEN 2000
#define UNSUPPORTED_MSG "Command not supported\n"
#define INVALID_MSG "Incorrect input!\n"


/* =============================================================================
 * launched process information
 * =============================================================================
 */
typedef struct {
    pid_t PID;
    TIMER_T start;
    TIMER_T end;
    int wstatus;
} process_t;


/* =============================================================================
 * handleSIGCHLD
 * =============================================================================
 */
void handleSIGCHLD(int s);


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName);

/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]);

/* =============================================================================
 * handleServer
 *  handle server commands
 * =============================================================================
 */
int handleServer(char* command);


/* =============================================================================
 * handleClient
 *  handle client requests
 * =============================================================================
 */
int handleClient(char* command, int fserv);



/* =============================================================================
 * registerChild
 *  save new child on saved child list
 * =============================================================================
 */
void registerChild(vector_t* v, pid_t pid, TIMER_T startTime);


/* =============================================================================
 * printStatus
 *  display launched processes information
 * =============================================================================
 */
void printStatus();


/* =============================================================================
 * logClientInput
 *  display client requests (shows client identifier)
 * =============================================================================
 */
void logClientInput(int nargs, char** input);
