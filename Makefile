CFLAGS = -Wall -std=gnu99 -g -pthread -I./
CC = gcc
CC_O = gcc $(CFLAGS) -c
LDFLAGS=-lm

ADVNME = CircuitRouter-AdvShell
SRVNME = CircuitRouter-AdvShell
SRVPTH = $(ADVNME)/$(SRVNME)
SRVEXE = $(SRVPTH)
SRVSRC = $(SRVPTH).c
SRVSRCH = $(SRVPTH).h
SRVDEPH = ./lib/commandlinereader.h ./lib/vector.h $(ADVNME)/ServerUtil.h
SRVSRCO = $(SRVSRC:%.c=%.o)
SRVDEPO = $(SRVDEPH:%.h=%.o)

CLTNME = CircuitRouter-Client
CLTPTH = $(ADVNME)/$(CLTNME)
CLTEXE = $(CLTPTH)
CLTSRC = $(CLTPTH).c
CLTSRCH = $(CLTPTH).h
CLTDEPH = $(ADVNME)/ServerUtil.h
CLTSRCO = $(CLTSRC:%.c=%.o)
CLTDEPO = $(CLTDEPH:%.h=%.o)

PARNME = CircuitRouter-ParSolver
PARPTH = $(PARNME)/$(PARNME)
PAREXE = $(PARPTH)
PARSRC = $(PARPTH).c
PARSRCH =
PARDEPH_ = $(PARNME)/maze.h $(PARNME)/router.h ./lib/list.h
PARSRCO = $(PARSRC:%.c=%.o)
PARDEPO = $(PARDEPH_:%.h=%.o)
PARDEPH  = $(PARDEPH_) ./lib/timer.h ./lib/types.h

OBJECT = ./lib/commandlinereader.o ./lib/vector.o $(PARNME)/maze.o $(PARNME)/router.o ./lib/list.o
EXEC = $(SRVEXE) $(CLTEXE) $(PAREXE)


all: $(EXEC)

# Executable dependencies
$(SRVEXE): $(SRVSRCO) $(SRVDEPO) $(ADVNME)/ServerUtil.o
$(CLTEXE): $(CLTSRCO) $(CLTDEPO) $(ADVNME)/ServerUtil.o
$(PAREXE): $(PARSRCO) $(PARDEPO) $(PARNME)/coordinate.o $(PARNME)/grid.o ./lib/pair.o ./lib/queue.o ./lib/vector.o

# Server object dependencies
$(SRVSRCO): $(SRVSRC) $(SRVSRCH) $(SRVDEPH)

# Client object dependencies
$(CLTSRCO): $(CLTSRC) $(CLTSRCH) $(CLTDEPH)

# ParSolver object dependencies
$(PARSRCO): $(PARSRC) $(PARSRCH) $(PARDEPH)
$(PARNME)/router.o: $(PARNME)/router.c $(PARNME)/router.h $(PARNME)/coordinate.h $(PARNME)/grid.h ./lib/queue.h ./lib/vector.h
$(PARNME)/maze.o: $(PARNME)/maze.c $(PARNME)/maze.h $(PARNME)/coordinate.h $(PARNME)/grid.h ./lib/queue.h ./lib/list.h ./lib/pair.h ./lib/types.h ./lib/vector.h
$(PARNME)/grid.o: $(PARNME)/grid.c $(PARNME)/grid.h $(PARNME)/coordinate.h ./lib/types.h ./lib/vector.h
$(PARNME)/coordinate.o: $(PARNME)/coordinate.c $(PARNME)/coordinate.h ./lib/pair.h ./lib/types.h

# lib object dependencies
./lib/commandlinereader.o: ./lib/commandlinereader.c ./lib/commandlinereader.h
./lib/list.o: ./lib/list.c ./lib/list.h ./lib/types.h
./lib/vector.o: ./lib/vector.c ./lib/vector.h ./lib/types.h ./lib/utility.h
./lib/queue.o: ./lib/queue.c ./lib/queue.h ./lib/types.h
./lib/pair.o: ./lib/pair.c ./lib/pair.h
./lib/pair.o: ./lib/pair.c ./lib/pair.h


# object recipe
$(OBJECT):
	$(CC_O) $< -o $@

# executable recipe
$(EXEC):
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(CLTNME)/*.o $(CLTEXE)
	rm -f $(PARNME)/*.o $(PAREXE)
	rm -f $(SRVNME)/*.o $(SRVEXE)
	rm -f ./lib/*.o

