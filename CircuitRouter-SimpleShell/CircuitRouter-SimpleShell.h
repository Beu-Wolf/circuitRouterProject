#include <sys/types.h>

#define NARGS 3
#define BUFSIZE 100

typedef struct {
    pid_t PID;
    int wstatus;
} process_t;

int shellLoop(int maxChildren);
