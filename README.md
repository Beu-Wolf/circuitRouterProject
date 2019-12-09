# Circuit Router (October - November 2018)

Reuse and improvement of a previous Berkley Circuit Solver project in C language together with my [loyal project comrade](http://github.com/Beu-Wolf).
The project was developed in 3 stages:
 * **CircuitRouter-SimpleShell:** A simple shell (as the name suggests) that accepts commands from the stdin. Each valid command generates a child process that executes a sequential circuit router (CircuitRouter-SeqSolver). On the exit command, the shell waits for every launched process to finish and displays every child exit status and respective PID. It is possible to specify the maximum number of child processes running simultaneously (with a flag)

 * **CircuitRouter-ParSolver:** A Parallel Circuit Solver, using threads and shared memory. To ensure the routing correctness, we had to use mutexes. It is possible to specify the number of threads to use. We also developed a shell script (doTest.sh) to measure the threading speedups that were logged to a csv file (results directory)

 * **CircuitRouter-AdvShell:** An improved shell that acts as a server too. Using named pipes we were able to have clients sending requests and getting responses back. It was used Inter Process Communication (IPC) we were able to improve the shell performance

## Directory tree
``` bash
├── CircuitRouter-AdvShell
│   ├── CircuitRouter-AdvShell.c (server)
│   └── CircuitRouter-Client.c (client)
├── CircuitRouter-ParSolver
│   └── Parallel Solution
├── CircuitRouter-SeqSolver
│   └── Sequential Solution
├── CircuitRouter-SimpleShell
│   └── First developed shell
├── doTest.sh (test with single input files)
├── extra
│   ├── getCPUinfo.sh
│   └── testAll.sh (test all files in a given directory)
├── inputs
│   └── Inputs used for testing
├── lib
│   └── library files
├── Makefile
├── README.txt
└── results
    └── speedup result log files
```
## Usage
 * make
    -> compile the project
    -> make clean will clean the .o and executables

 * cd CircuitRouter-AdvShell && ./CircuitRouter-AdvShell (-h for options)
    -> run server (handle client requests and terminal commands)

 * cd CircuitRouter-AdvShell && ./CircuitRouter-Client
    -> run client

## What we've learned
During this project we learned a lot:

In SimpleShell, we created new processes and managed to synchronize the parent and the child processes, using wait and other OS tools.

In CircuitParSolver, we were able to increase the computation speed, since we were using threads and shared memory. The shared memory gave us a lot of trouble: We had to use mutexes in order to avoid race  conditions and ensure the program's correctness. We did our best to have fine grained locking, since we aimed to maximize the parallelism.

In AdvShell, we learned about named pipes (we weren't alowed to use sockets :( ) so we could establish client-server comunication. In this project the server and the clients were local, so we just handled Inter Process Comunication and data redirection. Since the server could handle multiple clients, it would be extremely inefficient if it was actively waiting for clients to connect. Facing this problem, we decided to use interruptions and alarms. We leared that those tools are difficult to deal with in C.

During the whole project duration we also made some Makefiles and shell scripts (to automatically test our programs) which was an enriching experience.
