Sample usage on a 166 core machine:

```
$ ./build.sh
g++ -Wall -std=c++17 -O3 -o sim.out sim.cc
$ time ./sim.out 4000000000
in_count: 3141592941 / 4000000000
approx_pi: 3.141592941

real    0m2.789s
user    0m2.783s
sys     0m0.002s
$ time ./sim.out 40000000000000 160
in_count: 31415926535164 / 40000000000000
approx_pi: 3.1415926535164

real    5m38.288s
user    888m4.566s
sys     0m16.604s
$ 
```

Able to complete ~4B samples in just a few seconds, because it's a small
number of CPU instructions per sample. With multithreading and a few
minutes, able to get 10 decimal places correct, which is already
approaching the limits of what floating point could handle. The code is
more accurate by using 64 bit scalars as a fixed point representation.
Details in sim.cc.
