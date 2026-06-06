#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

#ifdef POOZLE_WITH_BASELINES
#include <re2/re2.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#endif

namespace PzBench {

struct BenchResult {
  std::string name;
  double median_ms;
  double stddev_ms;
  double throughput_mbs;
  long long result_count;
};

void print_header(const std::string &section) {
  std::cout << "\n============================================================="
               "===========\n";
  std::cout << "  " << section << "\n";
  std::cout << "==============================================================="
               "=========\n";
}

// FIX: Added missing column headers for clean README output
void print_column_headers() {
  std::cout << std::left << std::setw(38) << "Benchmark Name" << std::setw(12)
            << "Median(ms)" << std::setw(12) << "StdDev(ms)" << std::setw(15)
            << "Throughput"
            << "Matches\n";
  std::cout << std::string(85, '-') << "\n";
}

void print_result(const BenchResult &r) {
  std::cout << std::fixed << std::setprecision(3) << std::left << std::setw(38)
            << r.name << std::setw(12) << r.median_ms << std::setw(12)
            << r.stddev_ms;
  if (r.throughput_mbs > 0)
    std::cout << std::setprecision(1) << std::setw(10) << r.throughput_mbs
              << " MB/s   ";
  else
    std::cout << std::setw(15) << "N/A";
  std::cout << r.result_count << "\n";
}

BenchResult time_it(const std::string &name, size_t bytes, int warmup, int runs,
                    std::function<long long()> fn) {
  for (int i = 0; i < warmup; ++i)
    fn();

  std::vector<double> timings;
  long long last_result = 0;

  for (int i = 0; i < runs; ++i) {
    auto t0 = std::chrono::high_resolution_clock::now();
    last_result = fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    timings.push_back(
        std::chrono::duration<double, std::milli>(t1 - t0).count());
  }

  std::sort(timings.begin(), timings.end());

  // Note: for runs < 10, trim will be 0, which is intended (we keep all data
  // for small sets)
  size_t trim = runs / 10;
  std::vector<double> trimmed(timings.begin() + trim, timings.end() - trim);

  double median = trimmed[trimmed.size() / 2];
  double sum = std::accumulate(trimmed.begin(), trimmed.end(), 0.0);
  double mean = sum / trimmed.size();
  double sq_sum =
      std::inner_product(trimmed.begin(), trimmed.end(), trimmed.begin(), 0.0);
  double stddev = std::sqrt(sq_sum / trimmed.size() - mean * mean);
  double throughput = (bytes > 0 && median > 0.0)
                          ? (bytes / 1024.0 / 1024.0) / (median / 1000.0)
                          : 0.0;

  return BenchResult{name, median, stddev, throughput, last_result};
}

} // namespace PzBench

#include "bench_fm_index.hpp"
#include "bench_nfa_regex.hpp"
#include "bench_scaling.hpp"
#include "corpus_gen.hpp"

int main(int argc, char *argv[]) {
  std::cout << "\n#############################################################"
               "###########\n";
  std::cout << "#  POOZLE BENCHMARKING SUITE (ZERO DEPENDENCY)\n";
  std::cout << "#  Build: Release (-O2)\n";
  std::cout << "###############################################################"
               "#########\n";

  PzBench::run_fm_index_benchmarks();
  PzBench::run_nfa_benchmarks();
  PzBench::run_scaling_benchmark();

  return 0;
}