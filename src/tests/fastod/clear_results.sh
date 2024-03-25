#! /usr/bin/bash

pycache='__pycache__'
results='results'
results_mem='results_mem'
myeasylog='myeasylog.log'

if [[ -d $pycache ]]; then
    rm -rf $pycache
fi

if [[ -d $results ]]; then
    rm -rf $results
fi

if [[ -d $results_mem ]]; then
    rm -rf $results_mem
fi

if [[ -f $myeasylog ]]; then
    rm $myeasylog
fi
