#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <queue>

#include "../include/FileScanner.h"
#include "../include/ModelScorer.h"
#include "../include/ModelWeights.h"

namespace fs = std::filesystem;
namespace Chrono = std::chrono;

using Clock = Chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Chrono::duration<double, std::milli>;

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

struct RankingResult
{
  std::vector<scoredFile> top_files;
  std::vector<scoredFile> bottom_files;
};

RankingResult rank_files(const std::vector<file_features> &files, ModelContext context, int n_samples)
{
  // allocate heaps for top N and bottom N scored files
  std::priority_queue<scoredFile, std::vector<scoredFile>, std::greater<scoredFile>> top_n_heap;
  std::priority_queue<scoredFile, std::vector<scoredFile>, std::less<scoredFile>> bottom_n_heap;

  // score each file and maintain top N and bottom N heaps
  for (const auto &file : files)
  {
    double score = calculate_file_score(file, context, false);
    scoredFile sfile{file, score};

    // --- Logic for Top N ---
    if (top_n_heap.size() < n_samples) // if we haven't filled the heap yet - push directly
      top_n_heap.push(sfile);

    else if (sfile > top_n_heap.top()) // if current score is greater than the smallest in the heap - insert it
    {
      top_n_heap.pop();
      top_n_heap.push(sfile);
    }

    // --- Logic for Bottom N ---
    if (bottom_n_heap.size() < n_samples) // if we haven't filled the heap yet - push directly
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

  return RankingResult{top_files, bottom_files};
}

void print_target_rankings(const RankingResult &ranking, const std::wstring &target_name, int n_samples)
{
  std::wcout << L"ANALYZING TARGET: " << target_name << std::endl;
  std::wcout << L"\n--- TOP " << n_samples << L" FILES ---" << std::endl;
  for (const auto &sf : ranking.top_files)
  {
    std::wcout << L"Score: " << std::fixed << std::setprecision(4) << sf.score
               << L" | " << sf.file.path << L"\\" << sf.file.name << std::endl;
  }

  std::wcout << L"\n--- BOTTOM " << n_samples << L" FILES ---" << std::endl;
  for (const auto &sf : ranking.bottom_files)
  {
    std::wcout << L"Score: " << std::fixed << std::setprecision(4) << sf.score
               << L" | " << sf.file.path << L"\\" << sf.file.name << std::endl;
  }
  std::wcout << std::endl;
}

void test_specific_file(wchar_t *TARGET_PATH, ModelContext context)
{
  file_features f = extract_file_features(fs::directory_entry(TARGET_PATH));
  print_file_features(f);

  double test_score = calculate_file_score(f, context, true);
}

const int N_SAMPLES = 10;

ModelContext GENERAL_CONTEXT = ModelContext{L"GENERAL", GENERAL_MODEL_WEIGHTS, GENERAL_MODEL_BIAS, GENERAL_HIGH_VAL_EXTS, GENERAL_JUNK_EXTS};
ModelContext FINANCE_CONTEXT = ModelContext{L"FINANCE", FINANCE_MODEL_WEIGHTS, FINANCE_MODEL_BIAS, FINANCE_HIGH_VAL_EXTS, FINANCE_JUNK_EXTS};
ModelContext HR_CONTEXT = ModelContext{L"HR", HR_MODEL_WEIGHTS, HR_MODEL_BIAS, HR_HIGH_VAL_EXTS, HR_JUNK_EXTS};
ModelContext IT_CONTEXT = ModelContext{L"IT", IT_MODEL_WEIGHTS, IT_MODEL_BIAS, IT_HIGH_VAL_EXTS, IT_JUNK_EXTS};

int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);

  std::vector<ModelContext> targets = {GENERAL_CONTEXT,
                                       FINANCE_CONTEXT,
                                       HR_CONTEXT,
                                       IT_CONTEXT};

  // const wchar_t *TARGET_PATH = L"D:\\downloads";
  const wchar_t *TARGET_PATH = L"C:\\MLData";
  std::wcout << L"Scanning directory: " << TARGET_PATH << std::endl;

  std::vector<file_features> files = scan_directory(TARGET_PATH);

  std::wcout << L"Found " << files.size() << L" files. Scoring now...\n"
             << std::endl;

  for (ModelContext &context : targets)
  {
    RankingResult result = rank_files(files, context, N_SAMPLES);
    print_target_rankings(result, context.target, N_SAMPLES);

    scoredFile highest_scored_file = result.top_files.front();
    calculate_file_score(highest_scored_file.file, context, true);
  }

  return 0;
}

/* int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);
  test_specific_file(L"D:\\downloads\\budget_2025.xlsx", FINANCE_CONTEXT);
  // test_specific_file(L"D:\\downloads\\Audio-Win10_Win11-6.0.9360.1\\Win64\\Realtek\\AlexaConfigExtension_9360\\alexaconfig.cat", GENERAL_CONTEXT);
  // test_specific_file(L"D:\\downloads\\Skeleton.tar\\Skeleton\\src\\main\\java\\bgu\\spl\\mics\\example\\services\\ExampleEventHandlerService.java", FINANCE_CONTEXT);
  // test_specific_file(L"D:\\downloads\\Skeleton.tar\\Skeleton\\src\\main\\java\\bgu\\spl\\mics\\application\\objects\\LiDarDataBase.java", HR_CONTEXT);
  // test_specific_file(L"D:\\downloads\\Audio-Win10_Win11-6.0.9360.1\\Win64\\Realtek\\AlexaConfigExtension_9360\\alexaconfig.cat", IT_CONTEXT);
  // test_specific_file(L"D:\\downloads\\Skeleton\\Skeleton\\.vscode\\settings.json", IT_CONTEXT);

  return 0;
} */