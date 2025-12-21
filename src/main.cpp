#include <filesystem>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <string>
#include <unordered_set>

namespace fs = std::filesystem;
namespace Chrono = std::chrono;

using Clock = Chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Chrono::duration<double, std::milli>;

struct file_features
{
  std::wstring path;
  std::wstring name;
  std::wstring extension;
  unsigned long file_size;
  unsigned long last_write_time;
  bool is_read_only;
};

file_features extract_file_features(const fs::directory_entry &entry)
{
  file_features features;
  std::error_code ec; // For error handling - if any operation fails we set default values

  features.path = entry.path().wstring();
  features.name = entry.path().filename().wstring();
  features.extension = entry.path().extension().wstring();
  features.file_size = static_cast<unsigned long>(entry.file_size(ec));
  features.file_size = ec ? 0 : features.file_size;

  auto last_write = entry.last_write_time(ec);
  if (!ec)
  {
    // todo: consider switching to system_clock if issues arise(python uses unix epoch time)
    //   Convert to seconds since last write
    auto toNow = fs::file_time_type::clock::now() - last_write;
    auto elapsedSec = Chrono::duration_cast<Chrono::seconds>(toNow).count();
    features.last_write_time = static_cast<unsigned long>(elapsedSec);
  }

  auto perms = entry.status(ec).permissions();
  features.is_read_only = ec ? false : (perms & fs::perms::owner_write) == fs::perms::none;

  return features;
}

double compute_recency_score(unsigned long last_write_time)
{
  double secs_in_day = 60.0 * 60.0 * 24.0;
  double age_days = static_cast<double>(last_write_time) / secs_in_day;
  return 1.0 / (age_days + 1.0);
}

bool is_valuable_ext(const std::wstring &extension)
{
  // todo: switch to json or config file
  static const std::unordered_set<std::wstring> high_value_exts = {
      L".pem", L".key", L".kdbx", L".xlsx", L".pdf", L".docx", L".wallet", L".sql"};

  return high_value_exts.find(extension) != high_value_exts.end();
}

// MurmurHash3_x86_32 implementation for consistency with sklearn
uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed)
{
  static const uint32_t c1 = 0xcc9e2d51;
  static const uint32_t c2 = 0x1b873593;
  uint32_t h1 = seed;
  const uint32_t *blocks = (const uint32_t *)key;
  int nblocks = len / 4;

  for (int i = 0; i < nblocks; i++)
  {
    uint32_t k1 = blocks[i];
    k1 *= c1;
    k1 = (k1 << 15) | (k1 >> 17);
    k1 *= c2;
    h1 ^= k1;
    h1 = (h1 << 13) | (h1 >> 19);
    h1 = h1 * 5 + 0xe6546b64;
  }

  const uint8_t *tail = (const uint8_t *)(key + nblocks * 4);
  uint32_t k1 = 0;
  switch (len & 3)
  {
  case 3:
    k1 ^= tail[2] << 16;
  case 2:
    k1 ^= tail[1] << 8;
  case 1:
    k1 ^= tail[0];
    k1 *= c1;
    k1 = (k1 << 15) | (k1 >> 17);
    k1 *= c2;
    h1 ^= k1;
  }

  h1 ^= len;
  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;
  return h1;
}

// todo: --- MODEL PARAMETERS (To be updated with real weights) ---
const double MODEL_BIAS = -2.5;                     // Example bias
const std::vector<double> MODEL_WEIGHTS(2051, 0.1); // Placeholder: 3 + 1024 + 1024

// function iterates over trigrams of input string, hashes them, and accumulates weights
double accumulate_hashing_weights(const std::wstring &w_input, int weight_offset)
{
  //todo: could cause loss of data
  // Convert to UTF-8 string to match Python hashing 
  std::string input(w_input.begin(), w_input.end());
  if (input.length() < 3)
    return 0.0;

  double hashed_weight_sum = 0.0;

  for (size_t i = 0; i <= input.length() - 3; ++i)
  {
    std::string trigram = input.substr(i, 3);
    uint32_t h = murmur3_32(trigram.c_str(), 3, 0);
    int feature_index = h % 1024;
    hashed_weight_sum += MODEL_WEIGHTS[weight_offset + feature_index];
  }

  return hashed_weight_sum;
}

double calculate_file_score(const file_features &features)
{
  double z = MODEL_BIAS;

  double recency_score = compute_recency_score(features.last_write_time);
  double size_logged = std::log1p(features.file_size);
  double valuable_ext = is_valuable_ext(features.extension) ? 1.0 : 0.0;

  z += (MODEL_WEIGHTS[0] * recency_score);
  z += (MODEL_WEIGHTS[1] * size_logged);
  z += (MODEL_WEIGHTS[2] * valuable_ext);

  // Add Hashing Features (Weights 3-1026 for name, 1027-2050 for path)
  z += accumulate_hashing_weights(features.name, 3);
  z += accumulate_hashing_weights(features.path, 1027);

  //  Sigmoid Activation: 1 / (1 + e^-z)
  return 1.0 / (1.0 + std::exp(-z));
}

void print_file_features(const file_features &features)
{
  std::wcout << L"[SCAN] " << features.path << std::endl;
  std::wcout << L"  - Name: " << features.name << std::endl;
  std::wcout << L"  - Type: " << features.extension << std::endl;
  std::wcout << L"  - Size: " << features.file_size << L" bytes\n";
  std::wcout << L"  - Mod Time: " << features.last_write_time << L" seconds ago\n";
  std::wcout << L"  - Read Only: " << (features.is_read_only ? L"Yes" : L"No") << std::endl;
  std::wcout << L"-----------------------------------\n";
}

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

void scan_directory(std::wstring target_path)
{
  std::error_code ec;
  auto options = fs::directory_options::skip_permission_denied;

  if (!fs::exists(target_path, ec))
  {
    std::wcerr << L"Target directory does not exist: " << target_path << std::endl;
    return;
  }

  // METRICS START
  size_t total_files = 0;
  auto start_time = Clock::now();

  try
  {
    std::wcout << L"Starting recursive scan of: " << target_path << std::endl;

    for (const auto &entry : fs::recursive_directory_iterator(target_path, options, ec))
    {
      if (ec)
      {
        ec.clear();
        continue;
      }

      try
      {
        if (entry.is_regular_file(ec)) // skip directories, sockets, etc.
        {
          file_features features = extract_file_features(entry);
          print_file_features(features);
          double score = calculate_file_score(features);
          std::wcout << L"  - file Score: " << score << std::endl;
          std::wcout << L"===================================\n";

          total_files++;
        }
      }
      catch (const fs::filesystem_error &e)
      {
        std::wcerr << L"Error reading specific file: " << e.what() << std::endl;
      }
    }
  }
  catch (const std::exception &e)
  {
    std::wcerr << L"Critical Scan Error: " << e.what() << std::endl;
  }

  // METRICS END and REPORT
  auto end_time = Clock::now();

  report_scan_metrics(total_files, start_time, end_time);
}

int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);
  const wchar_t *path = L"C:\\Users\\013ri\\OneDrive\\Documents\\schoolwork\\CyberProject\\CyberSecurityMLproject\\data";

  scan_directory(path);

  return 0;
}
