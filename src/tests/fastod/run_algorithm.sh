#! /usr/bin/bash

datasets='../../../test_input_data/od_norm_data/metanome/*.csv'

if [[ -n $1 ]]; then
	datasets=$@
fi

python3 run_algorithm.py -i ../../../cli/cli.py -d $datasets