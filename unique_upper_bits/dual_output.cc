#include <stdlib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>
#include <vector>

int main(int argc, char *argv[]) {
  std::random_device r1;
  std::mt19937_64 r{r1()};

  for (;;) {
    // Found for 14 bits: 0x6e9347f68876ba9b, 0x6216cfd41a1a2fd1
    // Found for 14 bits: 0x8a48bd00b51b2be9, 0x432a3fafbf7dfa2f
    // Found for 14 bits: 0x11631523fc87bd81, 0xc15e0ad9023f4969
    // Found for 15 bits: 0x11631523fc87bd81, 0xc15e0ad9023f4969
    // etc.

    /*
    WINNER @ 16 bits!!!!!!!!!!!!! 0xf01f39c13d6a7d81, 0x9683ac7883fc67b7
    */

    uint64_t factorA = r() | 1;
    uint64_t factorB = r() | 1;

    std::vector<bool> seen_plus1both;
    std::vector<bool> seen_plus2ao;
    std::vector<bool> seen_plus2bo;

    for (int bits = 1; bits <= 17; ++bits) {
      int nbits = 64 - bits;
      uint64_t max = uint64_t{1} << (2 * bits);
      seen_plus1both.assign(max << 2, false);
      seen_plus2ao.assign(max << 2, false);
      seen_plus2bo.assign(max << 2, false);

      int plus1both_collision = 0;
      int plus2ao_collision = 0;
      int plus2bo_collision = 0;

      for (uint64_t i = 0; i < max; ++i) {
        uint64_t a = i * factorA;
        uint64_t b = i * factorB;

        uint64_t v = ((a >> (nbits - 1)) << (bits + 1)) | (b >> (nbits - 1));
        plus1both_collision += seen_plus1both[v];
        seen_plus1both[v] = true;

        v = a >> (nbits * 2 - 2);
        plus2ao_collision += seen_plus2ao[v];
        seen_plus2ao[v] = true;

        v = b >> (nbits * 2 - 2);
        plus2bo_collision += seen_plus2bo[v];
        seen_plus2bo[v] = true;
      }

      if (plus1both_collision || plus2ao_collision || plus2bo_collision) break;
      if (bits >= 14) {
        fprintf(stderr, "Found for %d bits: 0x%lx, 0x%lx\n", bits, factorA, factorB);
      }
      if (bits >= 16) {
        fprintf(stderr, "WINNER @ %d bits!!!!!!!!!!!!! 0x%lx, 0x%lx\n", bits, factorA, factorB);
      }
    }
  }
  return 0;
}
