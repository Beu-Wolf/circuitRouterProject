#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "CircuitRouter-SimpleShell.h"
#include "lib/commandlinereader.h"
#include "lib/list.h"

#define PROMPT "[CrctRtr-SmplShll]$ "

int main(int argc, char** argv) {
    int maxChildren;

    // Read command line arguments
    maxChildren = 0;
    if(argc > 1) {
        maxChildren = atoi(argv[1]); 
    }

    return shellLoop(maxChildren);
}

int shellLoop(int const maxChildren) {
    // process counter
    int availableChildren = maxChildren;

    // command buffers
    char nargs, status = 1;
    char* line;
    char** args;

    // process list
    pid_t pid;
    int wstatus;
    list_t* procList;
    list_iter_t listIter;
    process_t* procInfo;


    procList = list_alloc(NULL);

    do {
        // line prompt
        printf(PROMPT);
        
        // alloc buffers
        line = (char*)malloc(sizeof(char) * BUFSIZ);
        if(line == NULL) { perror("CircuitRouter-SimpleShell"); }

        args = (char**)malloc(sizeof(char*) * NARGS);
        if(args == NULL) {
            perror("CircuitRouter-SimpleShell");
            free(line);
        }

        // read line
        nargs = readLineArguments(0, args, NARGS, line, BUFSIZE);
        
        // execute command
        if(nargs > 0) {
            if(!strcmp(args[0], "run")) { // [COMMAND]: run
                if(nargs < 2) {
                    // invalid call
                    printf("You must specify a filename!\n");

                } else {
                    // valid call                   
                    if(maxChildren && availableChildren <= 0) {
                        // cannot fork another time
                        do {
                            // wait for child to terminate
                            pid = wait(&wstatus);
                            if (pid == -1) {
                                perror("waitpid");
                                exit(EXIT_FAILURE);
                            }
                        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

                        //add process info to list
                        procInfo = (process_t*) malloc(sizeof(process_t));
                        if(!procInfo){
                            perror("addProcessToList");
                            exit(EXIT_FAILURE);
                        }

                        procInfo->PID = pid;
                        procInfo->wstatus = wstatus;

                        list_insert(procList, (void*)procInfo);                        
                    }
                    availableChildren--;

                    // run new process
                    pid = fork();
                    if(pid < 0) {
                        printf("Error while forking");
                        exit(EXIT_FAILURE);
                    } else if(pid == 0) { // child process
                        execv("./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver", args);
                        exit(EXIT_FAILURE); // error during execv
                    }
                }

            } else if (!strcmp(args[0], "exit")) {  // [COMMAND]: exit
                // print finished children
                for(list_iter_reset(&listIter, procList); list_iter_hasNext(&listIter, procList); procInfo = (process_t*) (list_iter_next(&listIter, procList))) {
                    printf("CHILD EXITED (PID=%d; return %s)\n", procInfo->PID,
                        (WIFEXITED(procInfo->wstatus) && !WEXITSTATUS(procInfo->wstatus) && !WIFSIGNALED(procInfo->wstatus)) ? "OK" : "NOK");
                    free(procInfo);
                }

                // wait and print final children
                do {
                    pid = wait(&wstatus);
                    if(pid == -1){
                        break;
                    }
                    printf("CHILD EXITED (PID=%d; return %s)\n", pid, (WIFEXITED(wstatus) && !WEXITSTATUS(wstatus) && !WIFSIGNALED(wstatus)) ? "OK" : "NOK");
                } while ((!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus)) || pid > 0);
                printf("END.\n");
                
                status = 0;
            } else {
                printf("Unknown command.\n");
            }
        }
        

        // free buffers
        free(args);
        free(line);
    } while(status);
    
    list_free(procList);

    return EXIT_SUCCESS;
}
