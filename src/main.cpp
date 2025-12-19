#include <filesystem>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <string>

namespace fs = std::filesystem;

struct file_features
{
  std::wstring path;
  std::wstring name;
  std::wstring extension;
  uintmax_t file_size;
  uintmax_t last_write_time;
  bool is_read_only;
};

// --- DEDICATED METRICS FUNCTION ---
void report_scan_metrics(size_t total_files,
                         std::chrono::high_resolution_clock::time_point start_time,
                         std::chrono::high_resolution_clock::time_point end_time)
{
  std::chrono::duration<double, std::milli> duration_ms = end_time - start_time;

  double total_time_ms = duration_ms.count();
  double avg_time_per_file = (total_files > 0) ? (total_time_ms / total_files) : 0.0;

  std::wcout << L"\n========== SCAN REPORT ==========\n";
  std::wcout << L"Total Files Scanned: " << total_files << L"\n";
  std::wcout << L"Total Time Taken:    " << std::fixed << std::setprecision(2) << total_time_ms << L" ms\n";
  std::wcout << L"Average Time/File:   " << std::fixed << std::setprecision(4) << avg_time_per_file << L" ms\n";
  std::wcout << L"=================================\n";
}

file_features extract_file_features(const fs::directory_entry &entry)
{
  file_features features;
  std::error_code ec;

  const auto &p = entry.path();
  features.path = p.wstring();
  features.name = p.filename().wstring();
  features.extension = p.extension().wstring();
  features.file_size = entry.file_size(ec);
  if (ec)
    features.file_size = 0;

  auto last_write = entry.last_write_time(ec);
  if (!ec)
    features.last_write_time = last_write.time_since_epoch().count();
  else
    features.last_write_time = 0;

  auto status = entry.status(ec);
  if (!ec)
  {
    auto perms = status.permissions();
    features.is_read_only = (perms & fs::perms::owner_write) == fs::perms::none;
  }
  else
  {
    features.is_read_only = false;
  }

  return features;
}

void print_file_features(const file_features &features)
{
  std::wcout << L"[SCAN] " << features.path << L"\n";
  std::wcout << L"  - Name: " << features.name << L"\n";
  std::wcout << L"  - Type: " << features.extension << L"\n";
  std::wcout << L"  - Size: " << features.file_size << L" bytes\n";
  std::wcout << L"  - Mod Time: " << features.last_write_time << L"\n";
  std::wcout << L"  - Read Only: " << (features.is_read_only ? L"Yes" : L"No") << L"\n";
  std::wcout << L"-----------------------------------\n";
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
  auto start_time = std::chrono::high_resolution_clock::now();

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
        if (entry.is_regular_file(ec))
        {
          file_features features = extract_file_features(entry);
          print_file_features(features);
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
  auto end_time = std::chrono::high_resolution_clock::now();

  report_scan_metrics(total_files, start_time, end_time);
}

int main()
{
  _setmode(_fileno(stdout), _O_U16TEXT);
  const wchar_t *path = L"C:\\Users\\013ri\\OneDrive\\Documents\\schoolwork\\CyberProject\\CyberSecurityMLproject\\data";

  scan_directory(path);

  return 0;
}
