#include <FMIndex.hpp>
#include <NfaBuilder.hpp>
#include <NfaMatcher.hpp>
#include <RegexPostfix.hpp>
#include <RegexTokenizer.hpp>
#include <pz_analysis.hpp>
#include <pz_buffer.hpp>
#include <pz_core.hpp>
#include <pz_error.hpp>
#include <pz_std.hpp>
using PzBufferSPtr = std::shared_ptr<PzStd::PzBuffer>;
namespace PzStd {

/**
 * @brief Performs exact pattern search on the buffer.
 * @param pattern The exact string pattern to search for.
 * @param results Vector to store the positions where pattern is found or
 * results as per implementation.
 * @return true if search was successful, false otherwise.
 */
bool PzAnalysisExact::analyze(const std::string &pattern,
                              std::vector<size_t> &results) {
  results.clear();
  try {
    auto positions = locate(pattern);
    for (int pos : positions) {
      results.push_back(static_cast<size_t>(pos));
    }
    return !results.empty();
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Exact analysis failed: " + std::string(e.what()));
    return false;
  }
}

int PzAnalysisExact::count(const std::string &pattern) {
  PzBufferSPtr buffer = core_ ? core_->pz_buffer_sptr : nullptr;

  if (buffer == nullptr || pattern.empty()) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Invalid buffer or empty pattern");
    return 0;
  }

  try {
    FMIndex &fm = buffer->fm_index;
    return fm.count(pattern, pattern.size());
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Exact count failed: " + std::string(e.what()));
    return -1;
  }
}

std::vector<int> PzAnalysisExact::locate(const std::string &pattern) {
  PzBufferSPtr buffer = core_ ? core_->pz_buffer_sptr : nullptr;

  if (buffer == nullptr || pattern.empty()) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Invalid buffer or empty pattern");
    return {};
  }

  try {
    FMIndex &fm = buffer->fm_index;
    return fm.locate(pattern);
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Exact count failed: " + std::string(e.what()));
    return {};
  }
}

// ── Shared helper: builds NFA matcher from a regex pattern string
// ───────────── Defined as a lambda inside each function to keep it
// self-contained. If you find yourself copying it a third time, extract it to a
// free function.

/**
 * @brief Performs regex pattern search on the buffer.
 * @param pattern The regex pattern to search for.
 * @param results Vector to store the positions where pattern matches or results
 * as per implementation.
 * @return true if search was successful, false otherwise.
 */
bool PzAnalysisRegex::analyze(const std::string &pattern,
                              std::vector<size_t> &results) {
  PzBufferSPtr buffer = core_ ? core_->pz_buffer_sptr : nullptr;
  if (buffer == nullptr || pattern.empty()) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Invalid buffer or empty pattern");
    return false;
  }

  const std::string &text = buffer->get_raw_text();
  if (text.empty())
    return false;

  try {
    Tokenizer tokenizer(pattern);
    auto tokens = tokenizer.tokenize();
    auto postfix = Postfix::convert(tokens);
    NfaBuilder builder;
    State *start = builder.build(postfix);
    NfaMatcher matcher(start);

    auto matches = matcher.find_all(text);
    results.clear();
    for (const auto &m : matches) {
      results.push_back(static_cast<size_t>(m.start_pos));
    }
    return !results.empty();
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Regex analysis failed: " + std::string(e.what()));
    return false;
  }
}

int PzAnalysisRegex::count(const std::string &pattern) {
  PzBufferSPtr buffer = core_ ? core_->pz_buffer_sptr : nullptr;
  if (buffer == nullptr || pattern.empty()) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Invalid buffer or empty pattern");
    return 0;
  }

  const std::string &text = buffer->get_raw_text();
  if (text.empty())
    return 0;

  try {
    Tokenizer tokenizer(pattern);
    auto tokens = tokenizer.tokenize();
    auto postfix = Postfix::convert(tokens);
    NfaBuilder builder;
    State *start = builder.build(postfix);
    NfaMatcher matcher(start);

    return static_cast<int>(matcher.find_all(text).size());
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Regex count failed: " + std::string(e.what()));
    return -1;
  }
}

std::vector<int> PzAnalysisRegex::locate(const std::string &pattern) {
  PzBufferSPtr buffer = core_ ? core_->pz_buffer_sptr : nullptr;
  if (buffer == nullptr || pattern.empty()) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Invalid buffer or empty pattern");
    return {};
  }

  const std::string &text = buffer->get_raw_text();
  if (text.empty())
    return {};

  try {
    Tokenizer tokenizer(pattern);
    auto tokens = tokenizer.tokenize();
    auto postfix = Postfix::convert(tokens);
    NfaBuilder builder;
    State *start = builder.build(postfix);
    NfaMatcher matcher(start);

    auto matches = matcher.find_all(text);
    std::vector<int> positions;
    positions.reserve(matches.size());
    for (const auto &m : matches) {
      positions.push_back(m.start_pos);
    }
    return positions;
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Regex locate failed: " + std::string(e.what()));
    return {};
  }
}

/**
 * @brief Constructs a PzAnalysis object with the given shared pointer to
 * PzCore. This constructor takes ownership of the provided core pointer. If the
 * pointer is null, it reports an error.
 * @param core Shared pointer to a PzCore instance.
 */
PzAnalysis::PzAnalysis(PzCoreSPtr core) : core_(std::move(core)) {
  if (core_ == nullptr) {
    PzError::report_error(PzErrorType::PZ_INVALID_INPUT,
                          "Null PzCore provided");
  }
}

/**
 * @brief Factory method to create a PzAnalysis instance.
 * This static method creates and returns a PzAnalysis object by taking
 * ownership of the given shared pointer to PzCore.
 * @param core Shared pointer to a PzCore instance.
 * @return PzAnalysis A new PzAnalysis object initialized with the given core.
 */
PzAnalysisSPtr PzAnalysis::create(PzCoreSPtr core) {
  return PzAnalysisSPtr(new PzAnalysis(std::move(core)));
}

/**
 * @brief Performs analysis by selecting the appropriate implementation based on
 * the analysis type.
 * @param type The type of analysis to perform (exact or regex).
 * @param pattern The pattern string to search for.
 * @param results Vector to store the results of the analysis.
 * @return true if analysis was successful, false otherwise.
 */
bool PzAnalysis::performAnalysis(PzAnalysisType type,
                                 const std::string &pattern,
                                 std::vector<size_t> &results) {
  try {
    if (impl_ == nullptr || curr_type_ != type) {
      switch (type) {
      case PzAnalysisType::PZ_ANALYSIS_TYPE_EXACT:
        impl_ = std::make_unique<PzAnalysisExact>(core_);
        break;
      case PzAnalysisType::PZ_ANALYSIS_TYPE_REGEX:
        impl_ = std::make_unique<PzAnalysisRegex>(core_);
        break;
      default:
        PzError::report_error(PzErrorType::PZ_INVALID_ANALYSIS_TYPE,
                              "Unknown analysis type");
        return false;
      }
      curr_type_ = type;
    }
    return impl_->analyze(pattern, results);
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Analysis failed: " + std::string(e.what()));
    return false;
  }
}

int PzAnalysis::count(PzAnalysisType type, const std::string &pattern) {
  try {
    if (impl_ == nullptr || curr_type_ != type) {
      switch (type) {
      case PzAnalysisType::PZ_ANALYSIS_TYPE_EXACT:
        impl_ = std::make_unique<PzAnalysisExact>(core_);
        break;
      case PzAnalysisType::PZ_ANALYSIS_TYPE_REGEX:
        impl_ = std::make_unique<PzAnalysisRegex>(core_);
        break;
      default:
        PzError::report_error(PzErrorType::PZ_INVALID_ANALYSIS_TYPE,
                              "Unknown analysis type");
        return -1;
      }
      curr_type_ = type;
    }
    return impl_->count(pattern);
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Counting failed: " + std::string(e.what()));
    return -1;
  }
}

std::vector<int> PzAnalysis::locate(PzAnalysisType type,
                                    const std::string &pattern) {
  try {
    if (impl_ == nullptr || curr_type_ != type) {
      switch (type) {
      case PzAnalysisType::PZ_ANALYSIS_TYPE_EXACT:
        impl_ = std::make_unique<PzAnalysisExact>(core_);
        break;
      case PzAnalysisType::PZ_ANALYSIS_TYPE_REGEX:
        impl_ = std::make_unique<PzAnalysisRegex>(core_);
        break;
      default:
        PzError::report_error(PzErrorType::PZ_INVALID_ANALYSIS_TYPE,
                              "Unknown analysis type");
        return {};
      }
      curr_type_ = type;
    }
    return impl_->locate(pattern);
  } catch (const std::exception &e) {
    PzError::report_error(PzErrorType::PZ_ANALYSIS_FAILED,
                          "Locating failed: " + std::string(e.what()));
    return {};
  }
}

} // namespace PzStd