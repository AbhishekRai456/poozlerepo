#ifndef POOZLE_PZ_CORE_H
#define POOZLE_PZ_CORE_H

#include <pz_std.hpp>

namespace PzStd {
class PzCore : public std::enable_shared_from_this<PzCore> {
private:
  PzStd::PzBufferSPtr pz_buffer_sptr;
  PzStd::PzAnalysisSPtr pz_analysis_sptr;

  PzCore();
  explicit PzCore(PzBufferSPtr buffer);

  PzCore(const PzCore &) = delete;
  PzCore &operator=(const PzCore &) = delete;
  PzCore(PzCore &&other) noexcept;
  PzCore &operator=(PzCore &&other) noexcept;

  void init();
  PzBufferSPtr get_buffer();

public:
  // ── Factory ──────────────────────────────────────────────────────────────
  static PzCoreSPtr create();
  static PzCoreSPtr create(PzBufferSPtr buffer);

  // ── Data loading ─────────────────────────────────────────────────────────
  bool load_text(std::string_view text);
  bool load_file(const std::string &filename);

  // ── Exact search (FM-Index) ───────────────────────────────────────────────
  int count_exact(const std::string &pattern);
  std::vector<int> locate_exact(const std::string &pattern);

  // ── Regex search (NFA engine) ─────────────────────────────────────────────
  int count_regex(const std::string &pattern);
  std::vector<int> locate_regex(const std::string &pattern);

  friend class PzAnalysis;
  friend class PzAnalysisExact;
  friend class PzAnalysisRegex;
};
} // namespace PzStd

#endif // POOZLE_PZ_CORE_H