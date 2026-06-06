#pragma once
#include <NfaBuilder.hpp>
#include <NfaMatcher.hpp>
#include <RegexPostfix.hpp>
#include <RegexTokenizer.hpp>
#include <iostream>
#include <vector>

namespace PzBench {

void run_nfa_benchmarks() {
  print_header("BENCHMARK 2: NFA Regex Profiling & Throughput");

  std::string email_pattern = "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}";
  std::string valid_email = "test.user_123@example.co.uk ";

  // Corpus 1: The Bulk String (Log Scanning)
  std::string throughput_corpus;
  throughput_corpus.reserve(valid_email.size() * 100000);
  for (int i = 0; i < 100000; i++)
    throughput_corpus += valid_email;

  // Corpus 2: The Vector (High-Frequency Classification)
  std::vector<std::string> email_list(100000, "test.user_123@example.co.uk");

  std::cout << "  Pattern: " << email_pattern << "\n\n";
  print_column_headers();

  NfaBuilder builder;
  State *start_state =
      builder.build(Postfix::convert(Tokenizer(email_pattern).tokenize()));
  std::regex std_re(email_pattern);

  // TEST A: BULK SCANNING (MB/s)
  // Establish raw throughput of Poozle. std::regex omitted here to avoid unfair
  // sregex_iterator heap overhead.
  auto r_pz_bulk = time_it(
      "Poozle NFA (2.8MB Bulk Scan)", throughput_corpus.size(), 1, 5, [&]() {
        NfaMatcher m(start_state);
        // FIX: Explicitly cast to long long
        return (long long)m.find_all(throughput_corpus).size();
      });

  print_result(r_pz_bulk);

  // TEST B: CLASSIFICATION (Ops/sec)
  // Fair head-to-head comparison
  auto r_std_class = time_it("std::regex (100k Classify)", 0, 1, 5, [&]() {
    long long matches = 0;
    for (const auto &s : email_list) {
      if (std::regex_search(s, std_re))
        matches++;
    }
    return matches;
  });

  auto r_pz_class = time_it("Poozle NFA (100k Classify)", 0, 1, 5, [&]() {
    long long matches = 0;
    for (const auto &s : email_list) {
      NfaMatcher m(start_state);
      if (!m.find_all(s).empty())
        matches++;
    }
    return matches;
  });

  print_result(r_std_class);
  print_result(r_pz_class);

  // --- TEST C: REDOS IMMUNITY ---
  std::cout << "\n--- ReDoS Immunity: (a+)+b on aaa...a! ---\n";
  std::vector<int> sizes = {15, 20, 25, 28};

  for (int n : sizes) {
    std::string adversarial = CorpusGen::make_repetitive_corpus("a", n, "!");
    std::string pat = "(a+)+b";

    std::regex std_re_redos(pat);
    NfaBuilder b;
    State *pz_start = b.build(Postfix::convert(Tokenizer(pat).tokenize()));

    auto r_pz = time_it("Poozle NFA", 0, 1, 3, [&]() {
      NfaMatcher m(pz_start);
      return m.find_all(adversarial).empty() ? 0 : 1;
    });

    std::cout << "N=" << std::setw(2) << n << " | Poozle: " << std::setw(6)
              << r_pz.median_ms << " ms | std::regex: ";

    if (n <= 25) {
      auto r_std = time_it("std::regex", 0, 1, 3, [&]() {
        return std::regex_search(adversarial, std_re_redos) ? 1 : 0;
      });
      std::cout << std::setw(8) << r_std.median_ms << " ms";
    } else {
      std::cout << "SKIPPED (>60s hang) ";
    }

// Optional baseline comparison against Google RE2
#ifdef POOZLE_WITH_BASELINES
    RE2 re2_pat(pat);
    auto r_re2 = time_it("Google RE2", 0, 1, 3, [&]() {
      return RE2::PartialMatch(adversarial, re2_pat) ? 1 : 0;
    });
    std::cout << " | Google RE2: " << std::fixed << std::setprecision(3)
              << (r_re2.median_ms * 1000.0) << " µs\n";
#else
    std::cout << "\n";
#endif
  }
}
} // namespace PzBench