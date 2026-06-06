#pragma once
#include <random>
#include <string>
#include <vector>

namespace PzBench {

class CorpusGen {
public:
  // Natural word generator for FM-Index (Requires spatial clustering for BWT)
  static std::string make_large_corpus(size_t size_bytes,
                                       const std::string &pattern,
                                       size_t freq = 500) {
    static const std::vector<std::string> word_pool = {
        "the", "quick", "brown",     "fox",       "jumps",  "over",  "lazy",
        "dog", "data",  "structure", "algorithm", "search", "index", "string"};
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, word_pool.size() - 1);

    std::string corpus;
    corpus.reserve(size_bytes + 1024);
    size_t count = 0;
    while (corpus.size() < size_bytes) {
      if (count > 0 && count % freq == 0)
        corpus += pattern + " ";
      else
        corpus += word_pool[dist(rng)] + " ";
      ++count;
    }
    return corpus;
  }

  // Adversarial repetitive text for NFA State Explosion / ReDoS testing
  static std::string make_repetitive_corpus(const std::string &base_pattern,
                                            size_t repeats,
                                            const std::string &noise = "!") {
    std::string result;
    result.reserve((base_pattern.size() * repeats) + noise.size());
    for (size_t i = 0; i < repeats; ++i) {
      result += base_pattern;
    }
    result +=
        noise; // Add guaranteed non-match at the end to force backtracking
    return result;
  }
};

} // namespace PzBench