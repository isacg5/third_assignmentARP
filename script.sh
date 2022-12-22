#!/bin/bash
if ! [ -d "bin" ]; then
  mkdir bin
fi

if ! [ -d "out" ]; then
  mkdir out
else
    rm -rf out
fi

make
./bin/master