#pragma once
#include <string>
#include <vector>
#include <random>
#include <string_view>
#include <stdexcept>
#include <cstdint>
#include <utility>

class RandomStreamGen {
public:
  struct Range {
    std::size_t l = 0;
    std::size_t r = 0;
  };

  explicit RandomStreamGen(const std::uint64_t seed = 0xC0FFEEULL)
    : rng_(seed),
      lenDist_(1, 30),
      charDist_(0, static_cast<int>(alphabet().size() - 1)) {
  }

  std::vector<std::string> generate(std::size_t n) {
    data_.clear();
    data_.reserve(n);
    for (std::size_t i = 0; i < n; ++i) data_.push_back(genOne());
    return data_;
  }

  const std::vector<std::string> &data() const { return data_; }
  std::size_t size() const { return data_.size(); }

  Range prefixPercent(int percent) const {
    if (percent < 0 || percent > 100) throw std::invalid_argument("percent must be 0..100");
    std::size_t n = data_.size();
    std::size_t r = (n * static_cast<std::size_t>(percent)) / 100;
    return Range{0, r};
  }

  std::vector<Range> prefixesByStep(int stepPercent) const {
    if (stepPercent <= 0 || stepPercent > 100) throw std::invalid_argument("step must be 1..100");
    std::vector<Range> res;
    for (int p = 0; p <= 100; p += stepPercent) res.push_back(prefixPercent(p));
    if (res.back().r != data_.size()) res.push_back(prefixPercent(100));
    return res;
  }

  std::vector<std::string_view> view(Range rg) const {
    if (rg.l > rg.r || rg.r > data_.size()) throw std::out_of_range("bad range");
    std::vector<std::string_view> out;
    out.reserve(rg.r - rg.l);
    for (std::size_t i = rg.l; i < rg.r; ++i) out.emplace_back(data_[i]);
    return out;
  }

private:
  static const std::string &alphabet() {
    static const std::string a =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "-";
    return a;
  }

  std::string genOne() {
    const int len = lenDist_(rng_);
    std::string s;
    s.resize(static_cast<std::size_t>(len));
    const auto &a = alphabet();
    for (int i = 0; i < len; ++i) s[static_cast<std::size_t>(i)] = a[static_cast<std::size_t>(charDist_(rng_))];
    return s;
  }

  std::mt19937_64 rng_;
  std::uniform_int_distribution<int> lenDist_;
  std::uniform_int_distribution<int> charDist_;
  std::vector<std::string> data_;
};
