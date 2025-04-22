#!/bin/bash

set -e

if [ ! -d build ]; then
    mkdir "build"
fi

rm -rf "$(pwd)"/build/*

cmake -S . -B build && cmake --build build

if [ ! -d /usr/include/mymuduo ]; then
  mkdir "/usr/include/mymuduo"
fi

for header in include/*.h
do
#  echo "$header"
  cp "$header" /usr/include/mymuduo
done

cp build/lib/libmymuduo.so /usr/lib

ldconfig