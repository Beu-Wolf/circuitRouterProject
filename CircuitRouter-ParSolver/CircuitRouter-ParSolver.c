/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>

enum param_types {
    PARAM_BENDCOST = (unsigned char)'b',
    PARAM_XCOST    = (unsigned char)'x',
    PARAM_YCOST    = (unsigned char)'y',
    PARAM_ZCOST    = (unsigned char)'z',
};

enum param_defaults {
    PARAM_DEFAULT_BENDCOST = 1,
    PARAM_DEFAULT_XCOST    = 1,
    PARAM_DEFAULT_YCOST    = 1,
    PARAM_DEFAULT_ZCOST    = 2,
};

bool_t global_doPrint = TRUE;
char* global_inputFile = NULL;
char* file_name;
char* file_name_res_old;
long global_params[256]; /* 256 = ascii limit */
int nThreads = -1;



/* =============================================================================
 * fatal
 * =============================================================================
 */
static void fatal(char* msg) {
    // print error message and return failure
    perror(msg);
    exit(EXIT_FAILURE);
}

/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
    fprintf(stderr, "Usage: %s [options] <filename>\n", appName);
    fprintf(stderr, "\nOptions:                            (defaults)\n");
    fprintf(stderr, "    t <INT>    [t]hread number      [MANDATORY]\n");
    fprintf(stderr, "    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
    fprintf(stderr, "    p          [p]rint routed maze  (false)\n");
    fprintf(stderr, "    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
    fprintf(stderr, "    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
    fprintf(stderr, "    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
    fprintf(stderr, "    h          [h]elp message       (false)\n");
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
    global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
    global_params[PARAM_XCOST]    = PARAM_DEFAULT_XCOST;
    global_params[PARAM_YCOST]    = PARAM_DEFAULT_YCOST;
    global_params[PARAM_ZCOST]    = PARAM_DEFAULT_ZCOST;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]){
    long i;
    long opt;

    opterr = 0;

    setDefaultParams();

    if(argc < 3) {
    	opterr++;
    }

    while ((opt = getopt(argc, argv, "hb:px:y:z:t:")) != -1) {
        switch (opt) {
            case 'b':
            case 'x':
            case 'y':
            case 'z':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
          	case 't':	
          		nThreads = atoi(optarg);
          		if(nThreads > 0) 
          			break;
            case '?':
            case 'h':
            default:
                opterr++;
                break;
        }
    }

    if(optind == argc){
        printf("Missing file name\n");
        opterr++;
    } else{
        for (i = optind; i < argc-1; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    }
    if (opterr) {
        displayUsage(argv[0]);
    }

    file_name = argv[optind];
    file_name_res_old = (char*)malloc(sizeof(char)*(strlen(file_name)+9));
    if(file_name_res_old == NULL) fatal("allocating memory");

}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
    /*
     * Initialization
     */
	FILE *finput, *foutput;
    char* result = ".res";
    char* old = ".old";

    pthread_t* tids = NULL;
    
    parseArgs(argc, (char** const)argv);
 	
    finput = fopen(file_name, "r");
    if(finput == NULL) fatal("While opening file");

    // generate output filenames
    strcat(file_name, result);
    strcpy(file_name_res_old, file_name);
    strcat(file_name_res_old, old);

    if (access(file_name, F_OK) == 0){ //if .res already exists
        if(access(file_name_res_old, F_OK) == 0){  //if .old already exists
            if(remove(file_name_res_old))
                fatal("Error removing file");
        } 
        if(rename(file_name, file_name_res_old))
            fatal("Error renaming output file");
    }
   
    foutput = fopen(file_name, "a");
    if(foutput == NULL) {
        fclose(finput);
        fatal("While opening file");
    }

    // allocate tids
    tids = (pthread_t*) malloc(sizeof(pthread_t) * nThreads);
    assert(tids != NULL);

    maze_t* mazePtr = maze_alloc();
    assert(mazePtr);
    long numPathToRoute = maze_read(mazePtr, finput, foutput);
    long gridSize = grid_size(mazePtr->gridPtr);
    router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
                                       global_params[PARAM_YCOST],
                                       global_params[PARAM_ZCOST],
                                       global_params[PARAM_BENDCOST],
                                       gridSize);
    assert(routerPtr);
    list_t* pathVectorListPtr = list_alloc(NULL);
    assert(pathVectorListPtr);

    router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr};
    TIMER_T startTime;
    TIMER_READ(startTime);

    // create threads
    for(int i = 0; i < nThreads; i++) {
    	if(pthread_create(&tids[i], 0, (void *) (router_solve), (void*) &routerArg)) fatal("while creating thread");
    }

    // wait for all threads to finish
    for(int i = 0; i < nThreads; i++) {
    	if(pthread_join(tids[i], NULL)) fatal("while waiting for threads");
    }

    TIMER_T stopTime;
    TIMER_READ(stopTime);

    long numPathRouted = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        numPathRouted += vector_getSize(pathVectorPtr);
	}

    fprintf(foutput, "Paths routed    = %li\n", numPathRouted);
    fprintf(foutput, "Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));



    /*
     * Check solution and clean up
     */
    assert(numPathRouted <= numPathToRoute);
    bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, global_doPrint, foutput);
    assert(status == TRUE);
    fputs("Verification passed.", foutput);

    free(tids);
    maze_free(mazePtr);
    router_free(routerPtr);

    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        vector_t* v;
        while((v = vector_popBack(pathVectorPtr))) {
            // v stores pointers to longs stored elsewhere; no need to free them here
            vector_free(v);
        }
        vector_free(pathVectorPtr);
    }
    list_free(pathVectorListPtr);

    if(fclose(finput) || fclose(foutput)) fatal("while closing file");

    free(file_name_res_old);
    printf("Circuit Solved!\n");
    exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
