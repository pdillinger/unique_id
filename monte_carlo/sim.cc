#include <stdlib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>


uint64_t MurmurHash64A ( const uint64_t * data, int len, uint64_t seed )
{
  const uint64_t m = 0xc6a4a7935bd1e995;
  const int r = 47;

  uint64_t h = seed ^ (len * 8 * m);

  const uint64_t * end = data + len;

  while(data != end)
  {
    uint64_t k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

uint64_t random64() {
  static std::random_device r1;
  static std::mt19937_64 r2{r1()};
  std::array<uint64_t, 4> vals = {r1(), r2(), r1(), r2()};
  return MurmurHash64A(vals.data(), vals.size(), 12345);
}

void Go(size_t count, int players, size_t iters) {
  constexpr int kBits = 20;
  constexpr size_t kSize = size_t{1} << kBits;
  constexpr size_t kMask = kSize - 1;

  enum Approach {
    kRandom,
    kAddition,
    kXor,
    kMultiXor,
    kMultiplyAddition,
    kMultiplyXor,
  };

  std::vector<bool> v;
  for (auto approach : {kRandom, kAddition, kXor, kMultiXor, kMultiplyAddition, kMultiplyXor}) {
    size_t threshold_total = 0;
    size_t collision_iters = 0;
    for (uint64_t iter = 0; iter < iters; ++iter) {
      v.assign(kSize, false);
      bool collision = false;
      uint64_t ids = 0;
      for (int p = 0; !collision && (players < 0 || p < players); ++p) {
        size_t start = static_cast<size_t>(random64()) & kMask;
        for (size_t i = 0; i < count && !collision; ++i) {
          size_t pos;
          size_t mult_offset = static_cast<size_t>(
              (i * uint64_t{0xf01f39c13d6a7d81}) >> (64 - kBits));
          switch (approach) {
          case kRandom:
            pos = static_cast<size_t>(random64()) & kMask;
            break;
          case kAddition:
            pos = (start + i) & kMask;
            break;
          case kXor:
            pos = start ^ i;
            break;
          case kMultiXor:
            pos = start ^ (i % 20) ^ ((i / 20) << 8);
            break;
          case kMultiplyAddition:
            pos = (start + mult_offset) & kMask;
            break;
          case kMultiplyXor:
            pos = start ^ mult_offset;
            break;
          }
          if (v[pos]) {
            collision = true;
            ++collision_iters;
            threshold_total += ids;
          } else {
            v[pos] = true;
            ++ids;
          }
        }
      }
    }
    const char *name = approach == kAddition       ? "addition"
                       : approach == kXor          ? "xor"
                       : approach == kMultiXor      ? "multixor"
                       : approach == kMultiplyAddition ? "multiply-addition"
                       : approach == kMultiplyXor      ? "multiply-xor"
                                                   : "random";
    if (players < 0) {
      std::cerr << "Average threshold with " << name << ": "
                << 1.0 * threshold_total / iters << " / "
                << kSize << std::endl;
    } else {
      std::cerr << "Collision probability " << name << ": "
                << 1.0 * collision_iters / iters << std::endl;
    }
  }
  // Example output with non-random count=1000:
  // Average threshold with random: 1282.94 / 1048576
  // Average threshold with addition: 28684 / 1048576
  // Average threshold with xor: 39829.2 / 1048576
  // Average threshold with multixor: 27957.5 / 1048576
  // Average threshold with multiply-addition: 20409.5 / 1048576
  // Average threshold with multiply-xor: 2807.11 / 1048576
  //
  // Winner is xor because there's lowest chance of any overlap, but addition
  // is close. Multiplicative hash + addition is within a factor of two, or
  // 1-2 bits of uniqueness (1 structured, e.g. counter, or 2 unstructured,
  // e.g. hash). Note that the random case matches roughly sqrt(n) we would
  // expect from the Birthday paradox.
  //
  // Example output with non-random count=50:
  // Average threshold with random: 1282.83 / 1048576
  // Average threshold with addition: 6448.52 / 1048576
  // Average threshold with xor: 8003.43 / 1048576
  // Average threshold with multixor: 5857.4 / 1048576
  // Average threshold with multiply-addition: 4668.4 / 1048576
  // Average threshold with multiply-xor: 2061.64 / 1048576
}

int main(int argc, char *argv[]) {
  size_t count = 1000;
  int players = 10;
  size_t iters = 10000;
  if (argc > 1) {
    count = static_cast<size_t>(atoi(argv[1]));
  }
  if (argc > 2) {
    players = atoi(argv[2]);
  }
  if (argc > 3) {
    iters = static_cast<size_t>(atoi(argv[3]));
  }
  Go(count, players, iters);
}
