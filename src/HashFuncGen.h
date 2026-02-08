#pragma once
#include <cstdint>
#include <string_view>
#include <random>
#include <vector>

class HashFuncGen {
public:
  class HashFunc {
  public:
    explicit HashFunc(std::uint64_t seed) : seed_(seed) {
    }

    std::uint32_t operator()(std::string_view s) const {
      std::uint64_t x = fnv1a64(s) ^ seed_;
      x = splitmix64(x);
      return static_cast<std::uint32_t>(x);
    }

  private:
    std::uint64_t seed_;

    static std::uint64_t fnv1a64(std::string_view s) {
      std::uint64_t h = 14695981039346656037ULL;
      for (unsigned char c: s) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
      }
      return h;
    }

    static std::uint64_t splitmix64(std::uint64_t x) {
      x += 0x9E3779B97F4A7C15ULL;
      x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
      x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
      return x ^ (x >> 31);
    }
  };

  explicit HashFuncGen(std::uint64_t seed = 0xBADC0DEULL) : rng_(seed) {
  }

  HashFunc make() {
    return HashFunc(rng_());
  }

  std::vector<HashFunc> makeMany(std::size_t k) {
    std::vector<HashFunc> hs;
    hs.reserve(k);
    for (std::size_t i = 0; i < k; ++i) hs.emplace_back(make());
    return hs;
  }

private:
  std::mt19937_64 rng_;
};
