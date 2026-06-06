#pragma once
#include <FMIndex.hpp>
#include <iomanip>
#include <iostream>
#include <pz_core.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace PzBench {

void run_scaling_benchmark() {
  print_header("BENCHMARK 3: Algorithmic Scaling (1MB -> 30MB)");

  std::vector<size_t> sizes = {
      1 * 1024 * 1024,  // 1 MB
      8 * 1024 * 1024,  // 8 MB
      15 * 1024 * 1024, // 15 MB
      30 * 1024 * 1024  // 30 MB (Stress Test)
  };
  std::string pattern = "algorithm";

  std::cout << "  Pattern: '" << pattern << "'\n\n";

  // Table Header
  std::cout << std::left << std::setw(10) << "Corpus" << std::setw(15)
            << "FM Build(ms)" << std::setw(20) << "Linear Scan (ms)"
            << std::setw(20) << "FM locate (ms)" << std::setw(12) << "Speedup"
            << std::setw(10) << "Matches"
            << "\n";
  std::cout << std::string(87, '-') << "\n";

  for (size_t sz : sizes) {
    std::string corpus = CorpusGen::make_large_corpus(sz, pattern);
    std::string size_label = std::to_string(sz / (1024 * 1024)) + "MB";

    auto r_linear = time_it("std::string::find", corpus.size(), 1, 5, [&]() {
      long long count = 0;
      size_t pos = 0;
      while ((pos = corpus.find(pattern, pos)) != std::string::npos) {
        ++count;
        pos++;
      }
      return count;
    });

    auto t0 = std::chrono::high_resolution_clock::now();
    FMIndex fm(corpus);
    auto t1 = std::chrono::high_resolution_clock::now();
    double build_ms =
        std::chrono::duration<double, std::milli>(t1 - t0).count();

    auto r_fm = time_it("FM-Index", corpus.size(), 1, 5, [&]() {
      // FIX 2: Explicit cast to long long
      return (long long)fm.locate(pattern).size();
    });

    // FIX 1: Use the result from time_it instead of calculating it a 3rd time
    long long match_count = r_fm.result_count;

    double speedup =
        (r_fm.median_ms > 0) ? r_linear.median_ms / r_fm.median_ms : 0.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << speedup << "x";
    std::string speedup_str = oss.str();

    // Table Row Output
    std::cout << std::left << std::setw(10) << size_label << std::fixed
              << std::setprecision(1) << std::setw(15) << build_ms
              << std::setprecision(3) << std::setw(20) << r_linear.median_ms
              << std::setw(20) << r_fm.median_ms << std::setw(12) << speedup_str
              << std::setw(10) << match_count << "\n";
  }

  // Multi-query amortization proof
  std::cout << "\n============================================================="
               "===========\n";
  std::cout
      << "  BENCHMARK 4: Multi-Query Amortization (1 Build, 100 Queries)\n";
  std::cout << "==============================================================="
               "=========\n";
  std::string mq_corpus =
      CorpusGen::make_large_corpus(10 * 1024 * 1024, pattern);

  std::cout << "  Building FM-Index for 10MB corpus... " << std::flush;
  FMIndex mq_fm(mq_corpus);
  std::cout << "Done.\n";

  std::vector<std::string> query_patterns = {
      "the",       "quick",      "brown",     "fox",       "data",
      "structure", "algorithm",  "search",    "index",     "string",
      "binary",    "linear",     "hash",      "tree",      "graph",
      "sort",      "merge",      "pointer",   "memory",    "cache",
      "latency",   "throughput", "buffer",    "compute",   "parallel",
      "thread",    "vector",     "deque",     "list",      "map",
      "set",       "pair",       "iterator",  "range",     "system",
      "kernel",    "process",    "signal",    "socket",    "file",
      "pipe",      "template",   "virtual",   "inline",    "static",
      "const",     "mutable",    "auto",      "class",     "struct",
      "enum",      "union",      "namespace", "typename",  "sizeof",
      "new",       "delete",     "throw",     "catch",     "try",
      "return",    "break",      "continue",  "if",        "else",
      "while",     "for",        "do",        "switch",    "case",
      "default",   "goto",       "void",      "bool",      "char",
      "short",     "long",       "float",     "double",    "unsigned",
      "signed",    "volatile",   "register",  "extern",    "friend",
      "operator",  "this",       "public",    "private",   "protected",
      "virtual",   "override",   "final",     "using",     "typedef",
      "decltype",  "constexpr",  "noexcept",  "nullptr",   "true",
      "false",     "algorithm",  "structure", "index",     "search",
      "pattern",   "corpus",     "build",     "locate",    "count",
      "rank",      "suffix",     "array",     "transform", "interval"};

  // Ensure exactly 100 patterns
  while (query_patterns.size() < 100)
    query_patterns.push_back("the");
  query_patterns.resize(100);

  auto r_fm_multi =
      time_it("FM-Index (100 queries)", mq_corpus.size(), 1, 5, [&]() {
        long long total = 0;
        for (const auto &p : query_patterns)
          total += (long long)mq_fm.locate(p).size();
        return total;
      });

  auto r_lin_multi =
      time_it("Linear scan (100 queries)", mq_corpus.size(), 1, 5, [&]() {
        long long total = 0;
        for (const auto &p : query_patterns) {
          size_t pos = 0;
          while ((pos = mq_corpus.find(p, pos)) != std::string::npos) {
            ++total;
            ++pos;
          }
        }
        return total;
      });

  std::cout << "  FM-Index  100 queries: " << r_fm_multi.median_ms << " ms\n";
  std::cout << "  Linear    100 queries: " << r_lin_multi.median_ms << " ms\n";
  std::cout << "  Speedup: " << std::fixed << std::setprecision(1)
            << (r_lin_multi.median_ms / r_fm_multi.median_ms) << "x\n";

  // CORRECTNESS ASSERTION (Manual Safe Loop)
  long long verify_fm_count = 0;
  long long verify_lin_count = 0;

  for (const auto &p : query_patterns) {
    verify_fm_count += mq_fm.locate(p).size();
    size_t pos = 0;
    while ((pos = mq_corpus.find(p, pos)) != std::string::npos) {
      ++verify_lin_count;
      ++pos;
    }
  }

  if (verify_fm_count != verify_lin_count) {
    std::cout << "  [!] COUNT MISMATCH: FM=" << verify_fm_count
              << " Linear=" << verify_lin_count << "\n";
  } else {
    std::cout << "  Correctness: FM and Linear precisely agree on "
              << verify_fm_count << " total matches.\n";
  }
}

} // namespace PzBench