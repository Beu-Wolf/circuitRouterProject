#!/bin/bash

# Usage: ./doTest <nThreads> <filepath> [printToConsole] [SuppressMake]


maxThreads=$1
filename=$2

PAREXE=CircuitRouter-ParSolver/CircuitRouter-ParSolver
SEQEXE=CircuitRouter-SeqSolver/CircuitRouter-SeqSolver
SHLEXE=CircuitRouter-SimpleShell/CircuitRouter-SimpleShell


# not enough arguments
if [ $# -lt 2 ]; then
	echo Usage: ./doTest.sh \<nThreads\> \<filePath\> \[printToConsole\]
	exit -1
fi

resultFile=$(basename ${filename}).speedups.csv
resultPath=results/${resultFile}


if [ ! -d results ]; then
	mkdir results
fi

if [ $# -lt 4 ] || [ ! -f ${SEQEXE} ] || [ ! -f ${PAREXE} ] || [ ! -f ${SHLEXE} ]; then
	make > extra/makelog.txt
	echo Make output in extra/makelog.txt
fi

#create result file
echo \#threads,exec_time,speedup > ${resultPath}

hr=$(date +%T)
./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver ${filename}

elapsedTime=$(grep -P '^Elapsed time\s+=\s+(\d+\.\d*)' ${filename}.res | grep -Po '\d+\.?\d*')
echo 1S,${elapsedTime},1 >> ${resultPath}
[ ! -z "$3" ] && echo -e \[${hr}\]: 1S'\t'${elapsedTime}'\t'1 #output to console if 3rd argument given

for i in $(eval echo {1..${maxThreads}}); do
	hr=$(date +%T)
	./CircuitRouter-ParSolver/CircuitRouter-ParSolver -t ${i} ${filename}
	parTime=$(grep -P '^Elapsed time\s+=\s+(\d+\.\d*)' ${filename}.res | grep -Po '\d+\.?\d*')
	speedup=$(echo "scale=6; ${elapsedTime}/${parTime}" | bc)
	echo ${i},${parTime},${speedup} >> ${resultPath}
	[ ! -z "$3" ] && echo -e \[${hr}\]: ${i}'\t'${parTime}'\t'${speedup} #output to console if 3rd flag given
done
