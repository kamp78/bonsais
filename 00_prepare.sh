#!/bin/sh

mkdir build
cd build
cmake ..
make
cd ..

tar xjvf enwiki-20150205.tar.bz2
