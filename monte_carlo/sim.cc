#include <stdlib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>

uint64_t fastrange64(uint64_t portion, uint64_t range) {
  return static_cast<uint64_t>((static_cast<__uint128_t>(portion) * range) >> 64);
}

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
  //return r2();
  std::array<uint64_t, 4> vals = {r1(), r2(), r1(), r2()};
  return MurmurHash64A(vals.data(), vals.size(), 12345);
}

void Go(size_t count, int players, size_t iters) {
  constexpr int kBits = 20;
  constexpr size_t kSize = size_t{1} << kBits;
  constexpr size_t kMask = kSize - 1;

  enum Approach {
    kRandom,
    kBucketFixed,
    kAdditionFixed,
    kXorFixed,
    kAdditionGeom,
    kXorGeom,
    kMultiXor,
    kMultiplyAddition,
    kMultiplyXor,
  };

  std::vector<bool> v;
  for (auto approach : {kRandom, kBucketFixed, kAdditionFixed, kXorFixed, kAdditionGeom, kXorGeom, kMultiXor, /*kMultiplyAddition, kMultiplyXor*/}) {
    size_t threshold_total = 0;
    size_t collision_iters = 0;
    for (size_t iter = 0; iter < iters; ++iter) {
      v.assign(kSize, false);
      size_t i = 0;
      size_t start = 0;
      const size_t max_ids = players < 0 ? SIZE_MAX : players * count;
      for (size_t ids = 0; ids < max_ids; ++ids) {
        if (i == 0) {
          start = static_cast<size_t>(random64()) & kMask;
          if (approach == kBucketFixed) {
            start = start / count * count;
          }
        }
        size_t pos;
        size_t mult_offset = static_cast<size_t>(
            (i * uint64_t{0xf01f39c13d6a7d81}) >> (64 - kBits));
        switch (approach) {
        case kBucketFixed:
        case kAdditionFixed:
        case kAdditionGeom:
          pos = (start + i) & kMask;
          break;
        case kRandom:
        case kXorFixed:
        case kXorGeom:
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
          ++collision_iters;
          threshold_total += ids;
          break;
        }
        v[pos] = true;
        ++i;
        switch (approach) {
        case kRandom:
          i = 0;
          break;
        case kAdditionFixed:
        case kBucketFixed:
        case kXorFixed:
          if (i == count) {
            i = 0;
          }
          break;
        default:
          // geometric distribution with mean `count`
          if (fastrange64(random64(), count) == 0) {
            i = 0;
          }
          break;
        }
      }
    }
    const char *name = approach == kAdditionFixed      ? "addition_fixed"
                       : approach == kAdditionGeom     ? "addition_geom"
                       : approach == kXorFixed         ? "xor_fixed"
                       : approach == kXorGeom          ? "xor_geom"
                       : approach == kBucketFixed      ? "bucket_fixed"
                       : approach == kMultiXor         ? "multixor"
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
