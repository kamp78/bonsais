#!/bin/sh

time_cmd="/usr/bin/time -l"
bench_exe="./build/bonsais"
file_name="enwiki-20150205"
num_nodes="110962030"

$time_cmd $bench_exe $file_name - 1 $num_nodes 0.8 5
$time_cmd $bench_exe $file_name - 1 $num_nodes 0.9 5
$time_cmd $bench_exe $file_name - 2 $num_nodes 0.8 6
$time_cmd $bench_exe $file_name - 2 $num_nodes 0.9 8