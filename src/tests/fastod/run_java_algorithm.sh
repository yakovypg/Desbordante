#! /usr/bin/bash

if [[ -z $1 ]]; then
	echo 'ERROR: Path to the Java algorithm implementation (bin directory) is not specified'
	exit 1
fi

java_alg_path=$1
datasets='../../../test_input_data/od_norm_data/metanome/*.csv'

if [[ -n $2 ]]; then
	datasets="${@:2}"
fi

python3 run_java_algorithm.py -i $java_alg_path -p leveretconey.fastod.Program -d $datasets