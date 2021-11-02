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
Found for 22 bits + 1 reserve: 0xd34952e2a348682f

Found for 23 bits + 1 reserve: 0xd8100e5ccfc93193
Found for 23 bits + 1 reserve: 0xe5ac3a34bf829d3d
Found for 23 bits + 1 reserve: 0x648040da4b5bd305
Found for 23 bits + 1 reserve: 0xd34007ded4c7fd6f
Found for 23 bits + 1 reserve: 0xb940102223616d05

Found for 26 bits + 1 reserve: 0x648040da4b5ca69b
Found for 26 bits + 1 reserve: 0x648040da4b5ca6de
Found for 26 bits + 1 reserve: 0xe5ac3a34bf82fb64

Found for 27 bits + 1 reserve: 0x648040da4b5ca438
Found for 27 bits + 1 reserve: 0x648040da4b5caa1b
Found for 28 bits + 1 reserve: 0xd34007ded4c945c9
Found for 29 bits + 1 reserve: 0xd34007ded4c945ef

Found for 29 bits + 1 reserve: 0xd34007ded4c94592
Found for 29 bits + 1 reserve: 0xd34007ded4c94594
Found for 29 bits + 1 reserve: 0xd34007ded4c94595
Found for 29 bits + 1 reserve: 0xd34007ded4c94596
Found for 29 bits + 1 reserve: 0xd34007ded4c94597
Found for 29 bits + 1 reserve: 0xd34007ded4c945ef
Found for 29 bits + 1 reserve: 0xd34007ded4c945f0
Found for 29 bits + 1 reserve: 0xd34007ded4c945f1
Found for 29 bits + 1 reserve: 0xd34007ded4c945f2
Found for 29 bits + 1 reserve: 0xd34007ded4c945f3
Found for 29 bits + 1 reserve: 0xd34007ded4c945f4
Found for 29 bits + 1 reserve: 0xd34007ded4c945f5
Found for 29 bits + 1 reserve: 0xd34007ded4c945f6

Found for 29 bits + 1 reserve: 0xd34007ded4c945ef
5×3044435078829274211 (2 distinct prime factors)
Found for 29 bits + 1 reserve: 0xd34007ded4c945f3
6547×137947×16854742451 (3 distinct prime factors)
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
    // Seek promising
    //uint64_t factor = r() | 1;

    // Refine
    /*
    uint64_t bases[] = {
      0xd8100e5ccfc93193,
      0xe5ac3a34bf829d3d,
      0x648040da4b5bd305,
      0xd34007ded4c7fd6f,
      0xb940102223616d05,
    };
    uint64_t factor = bases[r() % (sizeof(bases) / sizeof(*bases))];
    uint64_t modify_mask = (uint64_t{1} << (r() & 63)) - 1;
    factor ^= r() & modify_mask;
    */

    // Refine further
    uint64_t bases[] = {
      0x648040da4b5ca000,
      0xd34007ded4c94000,
    };
    uint64_t factor = bases[r() % (sizeof(bases) / sizeof(*bases))];
    factor ^= r() & 0xfff;

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

      if (nbits >= 29) {
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
