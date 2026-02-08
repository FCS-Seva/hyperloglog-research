#pragma once
#include <vector>
#include <cstdint>
#include <string_view>
#include <algorithm>
#include <cmath>

class HyperLogLog {
public:
  template<class Hash32>
  HyperLogLog(int B, Hash32 h) : B_(B), m_(1u << B), hash32_(wrap(h)), regs_(m_, 0) {
  }

  void reset() {
    std::fill(regs_.begin(), regs_.end(), 0);
  }

  void add(std::string_view s) {
    std::uint32_t x = hash32_(s);
    std::uint32_t idx = x >> (32 - B_);
    std::uint32_t w = x << B_;

    std::uint8_t rho;
    if (w == 0) rho = static_cast<std::uint8_t>((32 - B_) + 1);
    else rho = static_cast<std::uint8_t>(__builtin_clz(w) + 1);

    regs_[idx] = std::max(regs_[idx], rho);
  }

  double estimate() const {
    const double m = static_cast<double>(m_);
    const double alpha = alpha_m(m_);

    double z = 0.0;
    std::size_t V = 0;
    for (std::uint8_t r: regs_) {
      z += std::ldexp(1.0, -static_cast<int>(r));
      if (r == 0) ++V;
    }

    double E = alpha * m * m / z;

    if (E <= 2.5 * m && V > 0) {
      E = m * std::log(m / static_cast<double>(V));
    }

    constexpr double TWO32 = 4294967296.0;
    if (E > (TWO32 / 30.0)) {
      E = -TWO32 * std::log(1.0 - (E / TWO32));
    }

    return E;
  }

private:
  struct HashWrap {
    virtual ~HashWrap() = default;

    virtual std::uint32_t operator()(std::string_view) const = 0;
  };

  template<class H>
  struct HashWrapImpl final : HashWrap {
    H h;

    explicit HashWrapImpl(H hh) : h(hh) {
    }

    std::uint32_t operator()(std::string_view s) const override { return h(s); }
  };

  template<class H>
  static HashWrapImpl<H> wrap(H h) {
    return HashWrapImpl<H>(h);
  }

  static double alpha_m(std::size_t m) {
    if (m == 16) return 0.673;
    if (m == 32) return 0.697;
    if (m == 64) return 0.709;
    return 0.7213 / (1.0 + 1.079 / static_cast<double>(m));
  }

  int B_;
  std::size_t m_;
  HashWrapImpl<std::function<std::uint32_t(std::string_view)> > hash32_ = HashWrapImpl<std::function<std::uint32_t
    (std::string_view)> >(
    [](std::string_view) { return 0u; }
  );

  std::vector<std::uint8_t> regs_;
};
