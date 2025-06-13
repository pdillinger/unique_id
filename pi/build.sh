#! /bin/sh

GPP=g++
which g++.par 2>/dev/null >/dev/null && GPP=g++.par

CMD="$GPP -Wall -std=c++17 -O3 -o sim.out sim.cc"
echo "$CMD"
$CMD
