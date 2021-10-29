#include <stdlib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>
#include <vector>

// Golden ratio is surprisingly less effective than some other values
//uint64_t factor = 0x9683ac7883fc67b7;

/*
Found for 18 bits + 1 reserve: 0xc467de315a3b096f
Found for 18 bits + 1 reserve: 0xa41023ce03287f75
Found for 19 bits + 1 reserve: 0x9cd202d2c0fdcbab
Found for 20 bits + 1 reserve: 0x4827fb4ef68a8ddd
Found for 20 bits + 1 reserve: 0xb940080880108f5f
Found for 21 bits + 1 reserve: 0x7040e6b29b5c190f
Found for 21 bits + 1 reserve: 0xa1007dddd1928feb
*/

int main(int argc, char *argv[]) {
  int max_nbits = 32;
  int reserve_bits = 1;
  if (argc > 1) {
    max_nbits = static_cast<size_t>(atoi(argv[1]));
  }
  if (argc > 2) {
    reserve_bits = static_cast<size_t>(atoi(argv[2]));
  }

  std::random_device r1;
  std::mt19937_64 r{r1()};

  for (;;) {
    uint64_t factor = r() | 1;

    for (int nbits = 1; nbits < max_nbits; ++nbits) {
      size_t max = size_t{1} << nbits;
      std::vector<bool> seen;
      seen.resize(max << reserve_bits);

      int shift = 64 - nbits - reserve_bits;

      bool collision = false;
      for (size_t i = 0; i < max; ++i) {
        size_t alt = (i * factor) >> shift;
        if (seen[alt]) {
          collision = true;
          break;
        }
        seen[alt] = true;
      }

      if (collision) {
        break;
      }

      if (nbits >= 20) {
        std::cerr << "Found for " << nbits << " bits + " << reserve_bits
                  << " reserve: 0x" << std::hex << factor << std::dec
                  << std::endl;
      }
      if (nbits >= 32) {
        std::cerr << "WINNER!!!!" << std::endl;
      }
    }
  }
}
