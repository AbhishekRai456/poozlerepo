// smoke_test.cpp — full pipeline verification
#include <pz_core.hpp>
#include <iostream>
#include <vector>

// Helper to print a position vector
static void print_positions(const std::vector<int> &v) {
  std::cout << "[ ";
  for (int p : v) std::cout << p << " ";
  std::cout << "]";
}

int main() {
  const std::string corpus =
      "the cat sat on the mat and the cat wore a hat";
  //   0123456789...

  auto core = PzStd::PzCore::create();
  core->load_text(corpus);

  std::cout << "Corpus: \"" << corpus << "\"\n\n";

  // ── Exact search ────────────────────────────────────────────────────────
  {
    const std::string pat = "cat";
    int  cnt = core->count_exact(pat);
    auto pos = core->locate_exact(pat);
    std::cout << "[Exact] pattern=\"" << pat << "\"\n";
    std::cout << "        count=" << cnt << "  (expected: 2)\n";
    std::cout << "        positions=";
    print_positions(pos);
    std::cout << "  (expected: [ 4 30 ])\n\n";
  }

  {
    const std::string pat = "the";
    int  cnt = core->count_exact(pat);
    auto pos = core->locate_exact(pat);
    std::cout << "[Exact] pattern=\"" << pat << "\"\n";
    std::cout << "        count=" << cnt << "  (expected: 3)\n";
    std::cout << "        positions=";
    print_positions(pos);
    std::cout << "  (expected: [ 0 15 27 ])\n\n";
  }

  // ── Regex search ─────────────────────────────────────────────────────────
  {
    const std::string pat = "c.t";   // matches cat, any-char, t
    int  cnt = core->count_regex(pat);
    auto pos = core->locate_regex(pat);
    std::cout << "[Regex] pattern=\"" << pat << "\"\n";
    std::cout << "        count=" << cnt << "  (expected: 2)\n";
    std::cout << "        positions=";
    print_positions(pos);
    std::cout << "  (expected: [ 4 30 ])\n\n";
  }

  {
    const std::string pat = "[a-z]+at";   // word ending in 'at'
    int  cnt = core->count_regex(pat);
    auto pos = core->locate_regex(pat);
    std::cout << "[Regex] pattern=\"" << pat << "\"\n";
    std::cout << "        count=" << cnt << "  (expected: 4)\n";
    std::cout << "        positions=";
    print_positions(pos);
    std::cout << "  (expected: [ 4 8 19 34 ])\n\n";
  }

  return 0;
}