#!/bin/sh

if [ "$1" != "" ]; then
  echo "Compiling $1"
  make -sC src
  if ./src/pcl -f < $1 > a.asm; then
    clang -no-pie a.asm ./src/libpcl.a -lm
    rm a.asm
  else
    rm a.asm
    exit 1
  fi
else
  echo "Usage: ./compile.sh <input_file>"
fi
