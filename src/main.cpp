#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <string_view>
#include <cmath>
#include <algorithm>

#include "RandomStreamGen.h"
#include "HashFuncGen.h"
#include "HyperLogLog.h"

struct Stats {
  double mean = 0.0;
  double sd = 0.0;
};

int main() {
  const int B = 12;
  const int stepPercent = 5;
  const std::size_t N = 20000;
  const int streams = 30;

  HashFuncGen hgen(777);
  auto h = hgen.make();

  const int stepsCount = (100 / stepPercent) + 1;
  std::vector<double> sum(stepsCount, 0.0), sumsq(stepsCount, 0.0);
  std::vector<double> trueSum(stepsCount, 0.0);
  std::vector<std::size_t> processed(stepsCount, 0);

  std::ofstream outStream0("stream0.csv");
  outStream0 << "percent,processed,trueF0,estimateNt\n";

  for (int s = 0; s < streams; ++s) {
    RandomStreamGen gen(1000 + s);
    gen.generate(N);

    HyperLogLog hll(B, [&](std::string_view sv) { return h(sv); });

    std::unordered_set<std::string_view> exact;
    exact.reserve(static_cast<std::size_t>(N * 1.3));

    std::size_t last = 0;
    int stepIdx = 0;

    for (int p = 0; p <= 100; p += stepPercent, ++stepIdx) {
      std::size_t r = (N * static_cast<std::size_t>(p)) / 100;

      for (std::size_t i = last; i < r; ++i) {
        std::string_view sv(gen.data()[i]);
        exact.insert(sv);
        hll.add(sv);
      }
      last = r;

      const std::size_t trueF0 = exact.size();
      const double est = hll.estimate();

      sum[stepIdx] += est;
      sumsq[stepIdx] += est * est;
      trueSum[stepIdx] += static_cast<double>(trueF0);
      processed[stepIdx] = r;

      if (s == 0) {
        outStream0 << p << "," << r << "," << trueF0 << "," << est << "\n";
      }
    }
  }

  std::ofstream outStats("stats.csv");

  outStats << "percent,processed,meanNt,sdNt,meanTrueF0\n";

  for (int i = 0; i < stepsCount; ++i) {
    const int percent = i * stepPercent;
    const double m = sum[i] / streams;
    double v = (sumsq[i] / streams) - (m * m);
    if (v < 0) v = 0;
    const double sd = std::sqrt(v);
    const double mt = trueSum[i] / streams;

    outStats << percent << "," << processed[i] << "," << m << "," << sd << "," << mt << "\n";
  }

  std::cout << "OK: stream0.csv, stats.csv generated\n";
  return 0;
}
