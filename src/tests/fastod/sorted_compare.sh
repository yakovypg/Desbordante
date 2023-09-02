#!/bin/bash

if [[ $1 == '--help' ]] || [[ $1 == '-h' ]]; then
	echo 'First argument is the path to the first file.'
	echo 'Second argument is the path to the second file.'
	echo '[optional] Third argument is the postfix for temporary file names.'
	echo ''
	echo 'Example: $ bash sorted_compare.sh ./file1.txt ./file2.txt'
	exit 0
fi

if [[ -z $1 ]]; then
	echo 'ERROR: First file path is not specified'
	exit 1
elif [[ -z $2 ]]; then
	echo 'ERROR: Second file path is not specified'
	exit 1
fi

if [[ ! -f $1 ]]; then
	echo 'ERROR: First file not exists'
	exit 2
elif [[ ! -f $2 ]]; then
	echo 'ERROR: Second file not exists'
	exit 2
fi

file1=$1
file2=$2
postfix='__temp__'

if [[ -n $3 ]]; then
    postfix=$3
fi

sorted_file1="${file1}${postfix}"
sorted_file2="${file2}${postfix}"

if [[ -f $sorted_file1 ]]; then
	echo "ERROR: Temporary file ${sorted_file1} already exists"
	exit 3
elif [[ -f $sorted_file2 ]]; then
	echo "ERROR: Temporary file ${sorted_file2} already exists"
	exit 3
fi

sort $file1 -o $sorted_file1
sort $file2 -o $sorted_file2

difference=$(diff $sorted_file1 $sorted_file2)

if [[ -z $difference ]]; then
	echo 'INFO: Equal'
else
	echo 'INFO: Not equal'
	echo ''
	echo 'Difference:'
	echo "${difference}"
fi

if [[ -f $sorted_file1 ]]; then
	rm $sorted_file1
fi
if [[ -f $sorted_file2 ]]; then
	rm $sorted_file2
fi
