#! /bin/sh

GPP=g++
which g++.par 2>/dev/null >/dev/null && GPP=g++.par

for TARGET in single dual_input dual_output; do
  CMD="$GPP -Wall -std=c++11 -O3 -o ${TARGET}.out ${TARGET}.cc"
  echo "$CMD"
  $CMD
done
