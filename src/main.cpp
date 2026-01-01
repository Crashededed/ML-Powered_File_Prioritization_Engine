#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <queue>

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

void test_specific_file(wchar_t *TARGET_PATH)
{
  file_features f = extract_file_features(fs::directory_entry(TARGET_PATH));
  print_file_features(f);

  double test_score = calculate_file_score(f);
  std::wcout << L"\nTest File Score: " << test_score << std::endl;
}

const wchar_t *TARGET_PATH = L"D:\\downloads";
const int N_SAMPLES = 15;
/* 
int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);

  // scan directory
  std::wcout << L"Scanning directory: " << TARGET_PATH << std::endl;
  std::vector<file_features> files = scan_directory(TARGET_PATH);
  std::wcout << L"Found " << files.size() << L" files. Scoring now...\n"
             << std::endl;

  // allocate heaps for top N and bottom N scored files
  std::priority_queue<scoredFile, std::vector<scoredFile>, std::greater<scoredFile>> top_n_heap;
  std::priority_queue<scoredFile, std::vector<scoredFile>, std::less<scoredFile>> bottom_n_heap;

  // score each file and maintain top N and bottom N heaps
  for (const auto &file : files)
  {
    double score = calculate_file_score(file);
    scoredFile sfile{file, score};

    // --- Logic for Top N ---
    if (top_n_heap.size() < N_SAMPLES) // if we haven't filled the heap yet - push directly
      top_n_heap.push(sfile);

    else if (sfile > top_n_heap.top()) // if current score is greater than the smallest in the heap - insert it
    {
      top_n_heap.pop();
      top_n_heap.push(sfile);
    }

    // --- Logic for Bottom N ---
    if (bottom_n_heap.size() < N_SAMPLES) // if we haven't filled the heap yet - push directly
      bottom_n_heap.push(sfile);

    else if (sfile < bottom_n_heap.top()) // if current score is smaller than the largest in the heap - insert it
    {
      bottom_n_heap.pop();
      bottom_n_heap.push(sfile);
    }
  }

  std::vector<scoredFile> top_files;
  std::vector<scoredFile> bottom_files;

  while (!top_n_heap.empty())
  {
    top_files.push_back(top_n_heap.top());
    top_n_heap.pop();
  }
  std::reverse(top_files.begin(), top_files.end());

  while (!bottom_n_heap.empty())
  {
    bottom_files.push_back(bottom_n_heap.top());
    bottom_n_heap.pop();
  }
  std::reverse(bottom_files.begin(), bottom_files.end());

  std::wcout << L"       PAYLOAD SUMMARY (Top/Bot " << N_SAMPLES << L")" << std::endl;
  std::wcout << L"--- TOP " << N_SAMPLES << L" HIGH SIGNAL FILES ---" << std::endl;
  for (const auto &sf : top_files)
  {
    std::wcout << L"Score: " << sf.score << L" | " << sf.file.path << L"\\" << sf.file.name << std::endl;
  }

  std::wcout << L"\n--- BOTTOM " << N_SAMPLES << L" NOISE FILES ---" << std::endl;
  for (const auto &sf : bottom_files)
  {
    std::wcout << L"Score: " << sf.score << L" | " << sf.file.path << L"\\" << sf.file.name << std::endl;
  }

  return 0;
}
 */
int main()
{
  test_specific_file((wchar_t *)L"D:\\downloads\\SPL251-Assignment3-student-template\\SPL251-Assignment3-student-template\\server\\target\\classes\\bgu\\spl\\net\\api\\StompMessagingProtocol.class");
  return 0;
}