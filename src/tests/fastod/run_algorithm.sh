#! /usr/bin/bash

datasets='../../../test_input_data/od_norm_data/metanome/*.csv'

if [[ -n $1 ]]; then
	datasets=$1
fi

python3 run_algorithm.py -i ../../../build/target/Desbordante_run -d $datasets