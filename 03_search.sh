#!/bin/sh

echo_and_do() {
  echo "$1"
  eval "$1"
}

bench_exe="./build/bonsais"
file_name="sample.txt"
num_nodes="8575826"

echo_and_do "$bench_exe $file_name $file_name 1 $num_nodes 0.8 5"
echo_and_do "$bench_exe $file_name $file_name 1 $num_nodes 0.9 5"
echo_and_do "$bench_exe $file_name $file_name 2 $num_nodes 0.8 6"
echo_and_do "$bench_exe $file_name $file_name 2 $num_nodes 0.9 8"