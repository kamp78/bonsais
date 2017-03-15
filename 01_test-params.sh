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
file_name="enwiki-20150205"
num_nodes="110962030"

for lf in 0.8 0.9
do
  for w1 in `seq 4 10`
  do
    echo_and_do "$time_cmd $bench_exe $file_name - 2 $num_nodes $lf $w1";
  done
done
