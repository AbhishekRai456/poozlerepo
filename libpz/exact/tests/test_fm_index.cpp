#include "Alphabet.hpp"
#include "FMIndex.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int total = 0, passed = 0, failed = 0;

void check_count(const std::string &desc, const std::string &text,
                 const std::string &pattern, int expected_count) {
  total++;
  FMIndex fm(text);
  int got = fm.count(pattern, pattern.size());
  if (got == expected_count) {
    passed++;
    std::cout << "[PASS] " << desc << "\n";
  } else {
    failed++;
    std::cout << "[FAIL] " << desc << " | expected count=" << expected_count
              << " got=" << got << "\n";
  }
}

void check_locate(const std::string &desc, const std::string &text,
                  const std::string &pattern,
                  std::vector<int> expected_positions) {
  total++;
  FMIndex fm(text);
  std::vector<int> got = fm.locate(pattern);
  std::sort(got.begin(), got.end());
  std::sort(expected_positions.begin(), expected_positions.end());
  if (got == expected_positions) {
    passed++;
    std::cout << "[PASS] " << desc << "\n";
  } else {
    failed++;
    std::cout << "[FAIL] " << desc << " | expected positions: [";
    for (int p : expected_positions)
      std::cout << p << " ";
    std::cout << "] got: [";
    for (int p : got)
      std::cout << p << " ";
    std::cout << "]\n";
  }
}

void check_empty(const std::string &desc, const std::string &text,
                 const std::string &pattern) {
  total++;
  FMIndex fm(text);
  std::vector<int> got = fm.locate(pattern);
  if (got.empty()) {
    passed++;
    std::cout << "[PASS] " << desc << "\n";
  } else {
    failed++;
    std::cout << "[FAIL] " << desc << " | expected empty, got " << got.size()
              << " results\n";
  }
}

int main() {
  std::cout << "\n=== FM-Index Correctness Tests ===\n\n";

  // Basic count correctness
  check_count("Single occurrence count", "hello world", "world", 1);
  check_count("Multiple occurrence count", "banana", "an", 2);
  check_count("No occurrence count", "hello", "xyz", 0);
  check_count("Pattern equals text count", "abc", "abc", 1);
  check_count("Single char repeated count", "aaaa", "a", 4);
  check_count("Pattern longer than text", "ab", "abc", 0);
  check_count("Single char text match", "a", "a", 1);
  check_count("Single char text no match", "a", "b", 0);

  // Basic locate correctness - exact positions
  check_locate("Locate single match", "hello world", "world", {6});
  check_locate("Locate at start", "abcdef", "abc", {0});
  check_locate("Locate at end", "abcdef", "def", {3});
  check_locate("Locate two occurrences", "abcabc", "abc", {0, 3});
  check_locate("Locate single char multiple", "aaaa", "a", {0, 1, 2, 3});
  check_locate("Locate in banana", "banana", "an", {1, 3});
  check_locate("Locate pattern equals text", "abc", "abc", {0});
  check_locate("Locate first char", "hello", "h", {0});
  check_locate("Locate last char", "hello", "o", {4});
  check_locate("Locate middle char repeated", "abacaba", "a", {0, 2, 4, 6});

  // Edge cases - no match
  check_empty("No match different text", "hello", "xyz");
  check_empty("No match longer pattern", "ab", "abc");
  check_empty("No match empty-ish pattern", "abc", "d");
  check_empty("No match case sensitive", "Hello", "hello");

  // Correctness of count vs locate agreement
  // This verifies the two operations are internally consistent
  {
    total++;
    std::string text = "mississippi";
    std::string pat = "issi";
    FMIndex fm(text);
    int cnt = fm.count(pat, pat.size());
    int loc = (int)fm.locate(pat).size();
    if (cnt == loc) {
      passed++;
      std::cout << "[PASS] count/locate agree on 'mississippi'/'issi'\n";
    } else {
      failed++;
      std::cout << "[FAIL] count=" << cnt << " locate=" << loc << "\n";
    }
  }
  {
    total++;
    std::string text = "abababab";
    std::string pat = "ab";
    FMIndex fm(text);
    int cnt = fm.count(pat, pat.size());
    int loc = (int)fm.locate(pat).size();
    if (cnt == loc) {
      passed++;
      std::cout << "[PASS] count/locate agree on 'abababab'/'ab'\n";
    } else {
      failed++;
      std::cout << "[FAIL] count=" << cnt << " locate=" << loc << "\n";
    }
  }

  std::cout << "\n=== SUMMARY ===\n";
  std::cout << "Total: " << total << " | Passed: " << passed
            << " | Failed: " << failed << "\n";
  return failed > 0 ? 1 : 0;
}