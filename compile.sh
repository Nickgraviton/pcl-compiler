#!/bin/sh

if [ "$1" != "" ]
then
  echo "Compiling $1"
  make -C src
  ./src/pcl -f < $1 > a.asm || exit 1
  clang a.asm ./src/libpcl.a -lm
  rm a.asm
else
  echo "Usage: ./compile.sh <input_file>"
fi
