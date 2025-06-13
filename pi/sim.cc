#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

// Logically, a fixed-point value >= 0 and < 1. It is represented by a scalar
// that is implicitly divided by 2^64. This provides close to 64 bits of
// precision, about 19 decimal digits, while floating point only provides about
// 53 bits, about 16 decimal digits.
// NOTE: On usage, a tiny correction may be used to represent values more
// uniformly > 0 and < 1, the equivalent of adding 0.5 to the scalar, or 2^-65
// to the logical value.
using Fixed64 = uint64_t;

inline Fixed64 square(Fixed64 v) {
  // A slightly skewed but efficient version that rounds down
  // return static_cast<uint64_t>((__uint128_t{v} * v) >> 64);

  // A fair rounding version that may be slightly less efficient (or oddly more
  // efficient)
  auto square = __uint128_t{v} * v;
  return static_cast<uint64_t>(square >> 64) +
         ((static_cast<uint64_t>(square) >> 63) & 1U);
}

size_t compute_in_count(size_t samples, int seed) {
  // Random 64-bit primes with decent distribution properties
  constexpr uint64_t kPrime1 = 0x876f170be4f1fcb9U;
  constexpr uint64_t kPrime2 = 0xf0433a4aecda4c5fU;

  // (Order of primes intentionally swapped vs. later usage)
  Fixed64 a = static_cast<uint64_t>(seed) * kPrime2;
  Fixed64 b = static_cast<uint64_t>(seed) * kPrime1;

  // Count the number of times the point a,b falls in the unit circle (the
  // relevant quarter of it).
  size_t in_count = 0;

  // Rather than testing
  //   a0,b0
  //   a1,b1
  //   a2,b2
  //   ...
  // We can leverage our computed values more by testing in a stutter-step
  // manner:
  //   a0,b0
  //   a1,b0
  //   a1,b1
  //   a2,b1
  //   a2,b2
  //   ...
  assert((samples % 2) == 0);
  auto iters = samples / 2;

  // Compute initial a^2
  Fixed64 a2 = square(a);
  for (size_t i = 0; i < iters; i++) {
    // Compute b^2
    Fixed64 b2 = square(b);

    // Check & count whether a^2 + b^2 is less than 1
    in_count += (a2 < UINT64_MAX - b2);
    // These variants might be slower or faster, and with slight accuracy
    // variations or skews:
    // in_count += (a2 <= UINT64_MAX - b2);
    // in_count += (a2 <= -b2);

    // Linear (or pseudorandom, or full entropy random) updates to a and b
    // (Linear converges more quickly)
    a += kPrime1;
    b += kPrime2;

    // TODO: merge in source of randomness, not necessarily full entropy
    // state1 ^= ...;
    // state2 ^= ...;

    // Compute a^2
    a2 = square(a);

    // Check & count whether a^2 + b^2 is less than 1
    in_count += (a2 < UINT64_MAX - b2);
  }
  return in_count;
}

inline uint64_t random64() {
  static std::random_device r1;
  static std::mt19937_64 r2{r1()};
  return r2();
}

int main(int argc, char *argv[]) {
  size_t samples = 10000;
  int thread_count = 1;
  int seed = static_cast<int>(std::random_device{}());
  if (argc > 1) {
    samples = static_cast<size_t>(atoll(argv[1]));
  }
  if (argc > 2) {
    thread_count = atoi(argv[2]);
  }
  if (argc > 3) {
    seed = atoi(argv[3]);
  }
  size_t samples_per_thread = samples / thread_count / 2 * 2;
  samples = samples_per_thread * thread_count;

  std::vector<std::thread> threads;
  std::vector<size_t> in_counts(thread_count);
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back([samples_per_thread, seed, i, &in_counts]() {
      in_counts[i] = compute_in_count(samples_per_thread, seed + i);
    });
  }
  size_t in_count = 0;
  for (int i = 0; i < thread_count; ++i) {
    threads[i].join();
    in_count += in_counts[i];
  }

  std::cout << "in_count: " << in_count << " / " << samples << std::endl
            << "approx_pi: " << std::setprecision(16)
            << (static_cast<double>(in_count) / static_cast<double>(samples)) *
                   4.0
            << std::endl;
}
