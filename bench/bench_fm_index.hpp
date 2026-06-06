#pragma once
#include <FMIndex.hpp>
#include <iostream>
#include <pz_core.hpp>
#include <string>

namespace PzBench {

void run_fm_index_benchmarks() {
  print_header("BENCHMARK 1: FM-Index Exact Search (Locate)");

  std::string pattern = "algorithm";

  std::cout << "  [1/3] Generating 10MB Corpus... " << std::flush;
  std::string corpus = CorpusGen::make_large_corpus(10 * 1024 * 1024, pattern);
  std::cout << "Done.\n";

  std::cout << "  [2/3] Building FM-Index... " << std::flush;
  auto t0 = std::chrono::high_resolution_clock::now();
  FMIndex fm(corpus);
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Built in " << std::chrono::duration<double>(t1 - t0).count()
            << " sec.\n";

  std::cout << "  [3/3] Running Queries...\n\n";

  // Correctness Check: FM count vs FM locate size must agree
  long long fm_count_val = fm.count(pattern, pattern.size());
  long long fm_locate_val = (long long)fm.locate(pattern).size();
  if (fm_count_val != fm_locate_val) {
    std::cerr << "[!] FM-Index internal inconsistency: count=" << fm_count_val
              << " locate=" << fm_locate_val << "\n";
  }

  auto r_linear = time_it("std::string::find", corpus.size(), 2, 10, [&]() {
    long long count = 0;
    size_t pos = 0;
    while ((pos = corpus.find(pattern, pos)) != std::string::npos) {
      ++count;
      pos++;
    }
    return count;
  });

  // FIX: Using locate() for a more realistic world-load benchmark
  auto r_fm = time_it("Poozle FM-Index locate()", corpus.size(), 2, 10,
                      [&]() { return fm.locate(pattern).size(); });

  print_column_headers();
  print_result(r_linear);
  print_result(r_fm);
}
} // namespace PzBench