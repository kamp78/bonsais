#!/bin/sh

echo_and_do() {
  echo "$1"
  eval "$1"
}

if [ "$(uname)" = "Darwin" ]; then
  time_cmd="/usr/bin/time -l"
else
  time_cmd="/usr/bin/time -v"
fi
bench_exe="./build/bonsais"
file_name="sample.txt"
num_nodes="8575826"

echo_and_do "$time_cmd $bench_exe $file_name - 1 $num_nodes 0.8 5"
echo_and_do "$time_cmd $bench_exe $file_name - 1 $num_nodes 0.9 5"
echo_and_do "$time_cmd $bench_exe $file_name - 2 $num_nodes 0.8 6"
echo_and_do "$time_cmd $bench_exe $file_name - 2 $num_nodes 0.9 8"
