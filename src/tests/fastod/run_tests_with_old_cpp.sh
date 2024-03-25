#! /usr/bin/bash

if [[ -z $1 ]]; then
	echo 'ERROR: Path to the old C++ algorithm implementation (Desbordante_run) is not specified'
	exit 1
fi

old_alg_path=$1
datasets='../../../test_input_data/od_norm_data/metanome/*.csv'

if [[ -n $2 ]]; then
	datasets="${@:2}"
fi

python3 test_with_old_cpp.py -f ../../../cli/cli.py -s $old_alg_path -c ./sorted_compare.sh -d $datasets