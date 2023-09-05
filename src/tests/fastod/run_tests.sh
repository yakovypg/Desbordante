#! /usr/bin/bash
if [[ -z $1 ]]; then
	echo 'ERROR: Path to the Java algorithm implementation (bin directory) is not specified'
	exit 1
fi

java_alg_path=$1

python3 fastod_tester.py -c ../../../build/target/Desbordante_run -j $java_alg_path -p leveretconey.fastod.Program -C ./sorted_compare.sh -d ../../../test_input_data/od_norm_data/*.csv