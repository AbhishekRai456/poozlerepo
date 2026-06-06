#include <pz_analysis.hpp>
#include <pz_buffer.hpp>
#include <pz_core.hpp>
#include <pz_error.hpp>
#include <pz_std.hpp>

// ── Constructors ─────────────────────────────────────────────────────────────

PzStd::PzCore::PzCore() { this->pz_buffer_sptr = PzStd::PzBuffer::create(); }

PzStd::PzCore::PzCore(PzBufferSPtr buffer) { this->pz_buffer_sptr = buffer; }

// ── Two-phase init (shared_from_this safe here)
// ───────────────────────────────

void PzStd::PzCore::init() {
  this->pz_analysis_sptr = PzStd::PzAnalysis::create(shared_from_this());
}

// ── Factory
// ───────────────────────────────────────────────────────────────────

PzStd::PzCoreSPtr PzStd::PzCore::create() {
  PzCoreSPtr core(new PzCore());
  core->init();
  return core;
}

PzStd::PzCoreSPtr PzStd::PzCore::create(PzBufferSPtr buffer) {
  PzCoreSPtr core(new PzCore(buffer));
  core->init();
  return core;
}

// ── Private helpers
// ───────────────────────────────────────────────────────────

PzStd::PzBufferSPtr PzStd::PzCore::get_buffer() { return pz_buffer_sptr; }

// ── Data loading
// ──────────────────────────────────────────────────────────────

bool PzStd::PzCore::load_text(std::string_view text) {
  // PzCore is a friend of PzBuffer so direct member access is valid here.
  // We write text first so the regex engine can read it back via
  // get_raw_text(). PzBuffer::load_text does NOT touch the `text` member — it
  // only builds the FM-Index and word list — so this assignment is not
  // overwritten.
  // 1. Store the raw text for Regex
  pz_buffer_sptr->text = std::string(text);

  // 2. Let the buffer do its normal loading
  bool success = pz_buffer_sptr->load_text(std::string(text), true);

  // 3. THE NUKE: Forcefully build the FM-Index right here so it cannot be empty
  pz_buffer_sptr->fm_index = FMIndex(std::string(text));

  return success;
}

bool PzStd::PzCore::load_file(const std::string &filename) {
  // load_from_file already handles building both the word list
  // and the raw text member internally.
  return pz_buffer_sptr->load_from_file(filename);
}

// ── Exact search
// ──────────────────────────────────────────────────────────────

int PzStd::PzCore::count_exact(const std::string &pattern) {
  return pz_analysis_sptr->count(PzAnalysisType::PZ_ANALYSIS_TYPE_EXACT,
                                 pattern);
}

std::vector<int> PzStd::PzCore::locate_exact(const std::string &pattern) {
  return pz_analysis_sptr->locate(PzAnalysisType::PZ_ANALYSIS_TYPE_EXACT,
                                  pattern);
}

// ── Regex search
// ──────────────────────────────────────────────────────────────

int PzStd::PzCore::count_regex(const std::string &pattern) {
  return pz_analysis_sptr->count(PzAnalysisType::PZ_ANALYSIS_TYPE_REGEX,
                                 pattern);
}

std::vector<int> PzStd::PzCore::locate_regex(const std::string &pattern) {
  return pz_analysis_sptr->locate(PzAnalysisType::PZ_ANALYSIS_TYPE_REGEX,
                                  pattern);
}

// ── Move semantics
// ────────────────────────────────────────────────────────────

PzStd::PzCore::PzCore(PzCore &&other) noexcept {
  this->pz_buffer_sptr = std::move(other.pz_buffer_sptr);
  this->pz_analysis_sptr = std::move(other.pz_analysis_sptr);
}

PzStd::PzCore &PzStd::PzCore::operator=(PzCore &&other) noexcept {
  if (this != &other) {
    this->pz_buffer_sptr = std::move(other.pz_buffer_sptr);
    this->pz_analysis_sptr = std::move(other.pz_analysis_sptr);
  }
  return *this;
}