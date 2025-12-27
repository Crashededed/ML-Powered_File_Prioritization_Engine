#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <vector>

#include "../include/FileScanner.h"
#include "../include/ModelScorer.h"
namespace fs = std::filesystem;
namespace Chrono = std::chrono;

using Clock = Chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Chrono::duration<double, std::milli>;

// --- DEDICATED METRICS FUNCTION ---
void report_scan_metrics(size_t total_files, TimePoint start_time, TimePoint end_time)
{
  Duration duration_ms = end_time - start_time;

  double total_time_ms = duration_ms.count();
  double avg_time_per_file = (total_files > 0) ? (total_time_ms / total_files) : 0.0;

  std::wcout << L"\n========== SCAN REPORT ==========\n";
  std::wcout << L"Total Files Scanned: " << total_files << std::endl;
  std::wcout << L"Total Time Taken:    " << std::fixed << std::setprecision(2) << total_time_ms << L" ms\n";
  std::wcout << L"Average Time/File:   " << std::fixed << std::setprecision(4) << avg_time_per_file << L" ms\n";
  std::wcout << L"=================================\n";
}

int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);
  // const wchar_t *target_path = L"data";

  // // scan directory
  // std::wcout << L"Scanning directory: " << target_path << std::endl;
  // std::vector<file_features> files = scan_directory(target_path);
  // std::wcout << L"Found " << files.size() << L" files. Scoring now...\n" << std::endl;

  // // Iterate through files and score them
  // for (const auto &file : files)
  // {
  //   double score = calculate_file_score(file);

  //   // Filter output to keep it readable
  //   if (score > 0.5)
  //     std::wcout << L"[HIGH VALUE] " << file.name << L" (Score: " << score << L")" << std::endl;

  // }

  // Check the score of a specific file
  file_features test_file = {
      L"C:\\Users\\benjamin50\\Downloads", // path
      L"establish.key",                    // name
      L".key",                             // extension
      841,                                 // file_size
      1750679372,                          // last_write_time
      false                                // is_read_only
  };

  double test_score = calculate_file_score(test_file);
  std::wcout << L"\nTest File Score: " << test_score << std::endl;

  return 0;
}
