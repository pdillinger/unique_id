#include <stdlib.h>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>

void Go(size_t count) {
  std::mt19937_64 r{std::random_device()()};

  constexpr uint64_t kMaxIter = 100000;
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
    uint64_t threshold_total = 0;
    for (uint64_t iter = 0; iter < kMaxIter; ++iter) {
      v.assign(kSize, false);
      bool collision = false;
      do {
        size_t start = static_cast<size_t>(r()) & kMask;
        for (size_t i = 0; i < count && !collision; ++i) {
          size_t pos;
          size_t mult_offset = static_cast<size_t>(
              (i * uint64_t{0xf01f39c13d6a7d81}) >> (64 - kBits));
          switch (approach) {
          case kRandom:
            pos = static_cast<size_t>(r()) & kMask;
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
          } else {
            v[pos] = true;
            ++threshold_total;
          }
        }
      } while (!collision);
    }
    const char *name = approach == kAddition       ? "addition"
                       : approach == kXor          ? "xor"
                       : approach == kMultiXor      ? "multixor"
                       : approach == kMultiplyAddition ? "multiply-addition"
                       : approach == kMultiplyXor      ? "multiply-xor"
                                                   : "random";
    std::cerr << "Average threshold with " << name << ": "
              << 1.0 * threshold_total / kMaxIter << " / "
              << kSize << std::endl;
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
  if (argc > 1) {
    count = static_cast<size_t>(atoi(argv[1]));
  }
  Go(count);
}
