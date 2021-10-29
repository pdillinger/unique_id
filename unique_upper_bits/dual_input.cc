#include <stdlib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>
#include <vector>

// v = (a + b) >> nbits;
// Found for 13 bits: 0x25fe04cf2d59f4f9, 0x9be6a1a42be9f0f1
// Found for 13 bits: 0x28aeef89f5fbdc71, 0xe9da69bbd38ee173
// Found for 13 bits: 0xf3ad333dc48482e7, 0xb94a786cb0acbe1d
// Found for 13 bits: 0x4d735a668d710ab1, 0x10927fa06e07c1f1
// Found for 14 bits: 0x31567764611f2655, 0x2bcdab471453b2a3
// Found for 14 bits: 0x18b6e94f3f91e8d1, 0xd7bc9d177812ac81
// Found for 14 bits: 0x302c9dde701a999b, 0x9d01695c009d7333
// Found for 15 bits: 0xf1efe74fa07ea2d5, 0xc5f810c6b59bc67b
// Found for 15 bits: 0xee4d7b19e76275d9, 0x52074fc64dd99621
// Found for 15 bits: 0xe746abe4b5a69937, 0xddc94a84b21b6fd7
// Found for 15 bits: 0x8dc54657a099e39f, 0x11ec8f7d3ac521b1
// Found for 15 bits: 0xa480232cd2a71e6b, 0xdeb093abf22f1477
// Found for 17 bits: 0x3535acc8422972d1, 0xf11b2dc0e0a5161f
// 0x3535acc8422972d1 = 79613 * 489113 * 98463917 (3 distinct prime factors)
// 0xf11b2dc0e0a5161f (prime)

// v = (a ^ b) >> nbits;
// (Seems harder to find solutions)

int main(int argc, char *argv[]) {
  std::random_device r1;
  std::mt19937_64 r{r1()};

  for (;;) {
    uint64_t factorA = r() | 1U;
    uint64_t factorB = r() | 1U;

    std::vector<bool> seen_plus1both;
    std::vector<bool> seen_plus2ao;
    std::vector<bool> seen_plus2bo;

    for (int bits = 14; bits <= 17; ++bits) {
      uint64_t max = uint64_t{1} << (2 * bits);
      seen_plus1both.assign(max << 2, false);
      seen_plus2ao.assign(max << 2, false);
      seen_plus2bo.assign(max << 2, false);
      int nbits = 64 - (2 * bits) - 2;

      uint64_t half_max = uint64_t{1} << bits;

      int plus1both_collision = 0;
      int plus2ao_collision = 0;
      int plus2bo_collision = 0;

      for (uint64_t i = 0; (i < max) & !plus1both_collision & !plus2ao_collision & !plus2bo_collision; ++i) {
        uint64_t a = i * factorA;
        uint64_t b = i * factorB;

        uint64_t v = a >> nbits;
        plus2ao_collision += seen_plus2ao[v];
        seen_plus2ao[v] = true;

        v = b >> nbits;
        plus2bo_collision += seen_plus2bo[v];
        seen_plus2bo[v] = true;

        // dual input
        a = (i & (half_max - 1)) * factorA;
        b = (i >> bits) * factorB;

        v = (a + b) >> nbits;
        plus1both_collision += seen_plus1both[v];
        seen_plus1both[v] = true;
      }

      if (plus1both_collision || plus2ao_collision || plus2bo_collision) {
        break;
      }
      if (bits >= 15) {
        fprintf(stderr, "Found for %d bits: 0x%lx, 0x%lx\n", bits, factorA, factorB);
      }
      if (bits >= 17) {
        fprintf(stderr, "WINNER @ %d bits!!!!!!!!!!!!! 0x%lx, 0x%lx\n", bits, factorA, factorB);
      }
    }
  }
  return 0;
}
