#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void extract_and_print_features(const fs::directory_entry &entry)
{
  std::error_code ec;

  // 1. Get File Path
  std::cout << "[SCAN] " << entry.path().string() << "\n";

  // 2. Get File Name
  std::cout << "  - File Name: " << entry.path().filename().string() << "\n";

  // 3. Get File Extension/Type
  std::cout << "  - File Type: " << entry.path().extension().string() << "\n";

  // 4. Get File Size (Feature: Size)
  // Note: directory_entry caches this, making it faster than stat()
  uintmax_t file_size = entry.file_size(ec);
  if (!ec)
  {
    std::cout << "  - Size: " << file_size << " bytes\n";
  }

  // 5. Get Last Modification Time (Feature: Recency)
  auto last_write = entry.last_write_time(ec);
  if (!ec)
  {
    std::cout << "  - Mod Time: " << last_write.time_since_epoch().count() << "\n";
  }

  // 6. Check Permissions (Feature: Context)
  // std::filesystem perms are a bit complex to print, but easy to check logic against
  auto status = entry.status(ec);
  if (!ec)
  {
    auto perms = status.permissions();
    // Example: Check if read-only
    bool read_only = (perms & fs::perms::owner_write) == fs::perms::none;
    std::cout << "  - Read Only: " << (read_only ? "Yes" : "No") << "\n";
  }

  std::cout << "-----------------------------------\n";
}

void scan_directory(std::string target_path)
{
  std::error_code ec;
  auto options = fs::directory_options::skip_permission_denied;

  // Check if root exists before starting
  if (!fs::exists(target_path, ec))
  {
    std::cerr << "Target directory does not exist: " << target_path << std::endl;
    return;
  }

  try
  {
    std::cout << "Starting recursive scan of: " << target_path << std::endl;

    for (const auto &entry : fs::recursive_directory_iterator(target_path, options, ec))
    {
      if (ec)
      {
        // Skip locked directories silently
        ec.clear();
        continue;
      }

      try
      {
        if (entry.is_regular_file(ec))
        {
          // Pass the entry object directly for efficiency
          extract_and_print_features(entry);
        }
      }
      catch (const fs::filesystem_error &e)
      {
        std::cerr << "Error reading specific file: " << e.what() << std::endl;
      }
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Critical Scan Error: " << e.what() << std::endl;
  }
}

int main()
{
  const char *path = "data";

  scan_directory(path);

  return 0;
}
