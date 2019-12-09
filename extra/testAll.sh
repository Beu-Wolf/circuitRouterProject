#!/bin/bash
start=`date +%s`

if [ $# -lt 2 ]; then
	echo "Usage: ./testAll.sh <max threads> <input directory path>"
	exit -1
fi

for file in $2/*.txt; do
	echo "./doTest ${1} ${file}"
	./doTest.sh ${1} ${file} 1 1
done

end=`date +%s`
echo runtime = $((end-start)) seconds
