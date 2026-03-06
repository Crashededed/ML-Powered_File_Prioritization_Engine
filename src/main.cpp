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

// Define model contexts for each target with their respective weights, biases, and extension sets
ModelContext GENERAL_CONTEXT = ModelContext{L"GENERAL", GENERAL_MODEL_WEIGHTS, GENERAL_MODEL_BIAS, GENERAL_HIGH_VAL_EXTS, GENERAL_JUNK_EXTS};
ModelContext FINANCE_CONTEXT = ModelContext{L"FINANCE", FINANCE_MODEL_WEIGHTS, FINANCE_MODEL_BIAS, FINANCE_HIGH_VAL_EXTS, FINANCE_JUNK_EXTS};
ModelContext HR_CONTEXT = ModelContext{L"HR", HR_MODEL_WEIGHTS, HR_MODEL_BIAS, HR_HIGH_VAL_EXTS, HR_JUNK_EXTS};
ModelContext IT_CONTEXT = ModelContext{L"IT", IT_MODEL_WEIGHTS, IT_MODEL_BIAS, IT_HIGH_VAL_EXTS, IT_JUNK_EXTS};

struct RankingResult
{
  std::vector<scoredFile> top_files;
  std::vector<scoredFile> bottom_files;
};

// Ranks files based on their scores and returns the top N and bottom N files for a given target context
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

    // Logic for Top N
    if (top_n_heap.size() < n_samples) // if we haven't filled the heap yet - push directly
      top_n_heap.push(sfile);

    else if (sfile > top_n_heap.top()) // if current score is greater than the smallest in the heap - insert it
    {
      top_n_heap.pop();
      top_n_heap.push(sfile);
    }

    // Logic for Bottom N
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

  // Extract files from heaps into vectors and reverse to get correct order
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

// Utility function to print the top and bottom ranked files for a given target context
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

// Function to test scoring of a specific file for debugging purposes
void test_specific_file(wchar_t *TARGET_PATH, ModelContext context)
{
  file_features f = extract_file_features(fs::directory_entry(TARGET_PATH));
  print_file_features(f);

  double test_score = calculate_file_score(f, context, true);
}

// Function to report performance metrics in a clear format
void report_metrics(size_t total_files,
                    TimePoint start_discovery, TimePoint end_discovery,
                    TimePoint start_inference, TimePoint end_inference)
{
  double discovery_ms = Duration(end_discovery - start_discovery).count();
  double total_inference_ms = Duration(end_inference - start_inference).count();

  double avg_discovery_per_file = (total_files > 0) ? (discovery_ms / total_files) : 0.0;
  double avg_inference_per_target = total_inference_ms / 4;
  double inference_per_file_per_target = (total_files > 0) ? (avg_inference_per_target / total_files) : 0.0;

  std::wcout << L"\n========== PERFORMANCE BREAKDOWN ==========\n";
  std::wcout << L"Total Files Processed:   " << total_files << std::endl;
  std::wcout << L"--------------------------------------------\n";
  std::wcout << L"1. EXTRACTING (I/O & Hashing)\n";
  std::wcout << L"   Total Time:           " << std::fixed << std::setprecision(2) << discovery_ms << L" ms\n";
  std::wcout << L"   Avg per File:         " << std::fixed << std::setprecision(4) << avg_discovery_per_file << L" ms\n";
  std::wcout << L"--------------------------------------------\n";
  std::wcout << L"2. INFERENCE (ML Scoring - " << 4 << L" Targets)\n";
  std::wcout << L"   Total Inference Time: " << std::fixed << std::setprecision(2) << total_inference_ms << L" ms\n";
  std::wcout << L"   Avg per Target:       " << std::fixed << std::setprecision(2) << avg_inference_per_target << L" ms\n";
  std::wcout << L"   Avg per File/Target:  " << std::fixed << std::setprecision(6) << inference_per_file_per_target << L" ms\n";
  std::wcout << L"============================================\n";
}

int main(int argc, char* argv[])
{
  // Set console output to UTF-16 to properly display wide characters(hebrew chars, etc.)
  _setmode(_fileno(stdout), _O_U16TEXT);

  std::vector<ModelContext> targets = {GENERAL_CONTEXT, FINANCE_CONTEXT, HR_CONTEXT, IT_CONTEXT};

  std::wstring targetPath = L"./test_data";
  int topN = 10;

  // command-line argument parsing for --path and --top
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--path" && i + 1 < argc) {
      std::string p = argv[++i];
      targetPath = std::wstring(p.begin(), p.end());
    } 
    else if (arg == "--top" && i + 1 < argc) 
      topN = std::stoi(argv[++i]);
  }

  std::wcout << L"Target Path: " << targetPath << L" | Showing Top: " << topN << std::endl;

  if (!fs::exists(targetPath) || !fs::is_directory(targetPath))
  {
    std::wcout << L"Error: Specified path does not exist or is not a directory." << std::endl;
    return 1;
  }

  std::wcout << L"Scanning directory: " << targetPath << std::endl;

  // File discovery and feature extraction, metrics
  auto start_extraction = Clock::now();
  std::vector<file_features> files = scan_directory(targetPath);
  auto end_extraction = Clock::now();

  if (files.empty())
  {
    std::wcout << L"No files found in the specified directory." << std::endl;
    return 0;
  }

  std::wcout << L"Found " << files.size() << L" files. Scoring now...\n\n";

  // Inference and ranking per context, metrics
  auto start_inference = Clock::now();
  for (ModelContext &context : targets)
  {
    RankingResult result = rank_files(files, context, topN);
    print_target_rankings(result, context.target, topN);

    scoredFile highest_scored_file = result.top_files.front();
    calculate_file_score(highest_scored_file.file, context, true);
  }
  auto end_inference = Clock::now();

  report_metrics(files.size(), start_extraction, end_extraction, start_inference, end_inference);

  return 0;
}