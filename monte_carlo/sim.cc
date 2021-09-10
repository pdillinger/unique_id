#include <cstdint>
#include <iostream>
#include <cstring>
#include <random>

void Go() {
  std::mt19937_64 r{std::random_device()()};

  constexpr uint64_t kMaxIter = 100000;
  constexpr int kBits = 20;
  constexpr size_t kSize = size_t{1} << kBits;
  constexpr size_t kMask = kSize - 1;

  enum Approach {
    kRandom,
    kAddition,
    kXor,
    kMultAddition,
    kMultXor,
  };

  std::vector<bool> v;
  for (auto approach : {kRandom, kAddition, kXor, kMultAddition, kMultXor}) {
    uint64_t threshold_total = 0;
    for (uint64_t iter = 0; iter < kMaxIter; ++iter) {
      v.assign(kSize, false);
      bool collision = false;
      do {
        size_t start = static_cast<size_t>(r()) & kMask;
        size_t count;
        if (approach == kRandom) {
          count = 1;
        } else {
          count = 1000;
        }
        for (size_t i = 0; i < count && !collision; ++i) {
          size_t pos;
          size_t mult_offset = static_cast<size_t>(
              (i * uint64_t{0xf01f39c13d6a7d81}) >> (64 - kBits));
          if (approach == kAddition) {
            pos = (start + i) & kMask;
          } else if (approach == kMultAddition) {
            pos = (start + mult_offset) & kMask;
          } else if (approach == kMultXor) {
            pos = start ^ mult_offset;
          } else {
            pos = start ^ i;
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
                       : approach == kMultAddition ? "multaddition"
                       : approach == kMultXor      ? "multxor"
                                                   : "random";
    std::cerr << "Average threshold with " << name << ": "
              << 1.0 * threshold_total / kMaxIter << " / "
              << kSize << std::endl;
  }
  // Example output with non-random count=1000:
  // Average threshold with random: 1283.67 / 1048576
  // Average threshold with addition: 28653.2 / 1048576
  // Average threshold with xor: 39762.9 / 1048576
  // Average threshold with multaddition: 20369.2 / 1048576
  // Average threshold with multxor: 2810.48 / 1048576
  //
  // Winner is xor because there's lowest chance of any overlap, but addition
  // is close. Multiplicative hash + addition is within a factor of two, or
  // 1-2 bits of uniqueness (1 structured, e.g. counter, or 2 unstructured,
  // e.g. hash). Note that the random case matches roughly sqrt(n) we would
  // expect from the Birthday paradox.
  //
  // Example output with non-random count=50:
  // Average threshold with random: 1284.02 / 1048576
  // Average threshold with addition: 6460.97 / 1048576
  // Average threshold with xor: 8001.48 / 1048576
  // Average threshold with multaddition: 4665.97 / 1048576
  // Average threshold with multxor: 2057.52 / 1048576
}

int main(int /*argc*/, char */*argv*/[]) {
  Go();
}
