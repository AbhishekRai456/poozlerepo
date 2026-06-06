#include <chrono>
#include <iostream>
#include <pz_core.hpp>
#include <string>
#include <vector>

using namespace std::chrono;

void print_usage() {
  std::cout << "Usage:\n";
  std::cout << "  poozle search --exact \"pattern\" <file.txt>\n";
  std::cout << "  poozle search --regex \"pattern\" <file.txt>\n";
}

int main(int argc, char *argv[]) {
  // 1. Basic Argument Parsing
  if (argc != 5 || std::string(argv[1]) != "search") {
    print_usage();
    return 1;
  }

  std::string flag = argv[2];
  std::string pattern = argv[3];
  std::string filepath = argv[4];

  if (flag != "--exact" && flag != "--regex") {
    std::cerr << "Error: Invalid flag '" << flag
              << "'. Use --exact or --regex.\n";
    print_usage();
    return 1;
  }

  // 2. Initialize Poozle Core Engine
  auto core = PzStd::PzCore::create();

  // 3. Load File using the Engine's native file loader
  auto load_start = high_resolution_clock::now();
  if (!core->load_file(filepath)) {
    std::cerr << "Error: Engine could not open or read file '" << filepath
              << "'\n";
    return 1;
  }
  auto load_end = high_resolution_clock::now();

  std::cout << "[INFO] File loaded and indexed in "
            << duration_cast<milliseconds>(load_end - load_start).count()
            << " ms.\n";

  // 4. Execute Search
  std::vector<int> locations;
  auto search_start = high_resolution_clock::now();

  if (flag == "--exact") {
    locations = core->locate_exact(pattern);
  } else {
    locations = core->locate_regex(pattern);
  }

  auto search_end = high_resolution_clock::now();
  auto search_time =
      duration_cast<microseconds>(search_end - search_start).count();

  // 5. Output Results
  std::cout << "\n========================================\n";
  std::cout << "SEARCH RESULTS\n";
  std::cout << "========================================\n";
  std::cout << "Engine:      "
            << (flag == "--exact" ? "FM-Index (Exact)" : "Thompson NFA (Regex)")
            << "\n";
  std::cout << "Pattern:     '" << pattern << "'\n";
  std::cout << "Matches:     " << locations.size() << "\n";
  std::cout << "Search Time: " << search_time << " µs\n";
  std::cout << "========================================\n";

  int display_limit = std::min(static_cast<int>(locations.size()), 5);
  if (display_limit > 0) {
    std::cout << "First " << display_limit << " match offset(s): [ ";
    for (int i = 0; i < display_limit; ++i) {
      std::cout << locations[i] << (i == display_limit - 1 ? "" : ", ");
    }
    if (locations.size() > 5)
      std::cout << " ...";
    std::cout << " ]\n\n";
  }

  return 0;
}