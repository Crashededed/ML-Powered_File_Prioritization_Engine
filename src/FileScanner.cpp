#include "../include/FileScanner.h"
#include <iostream>
#include <chrono>

namespace Chrono = std::chrono;

// Internal helper to extract details from a single entry
file_features extract_file_features(const fs::directory_entry &entry)
{
    file_features features;
    std::error_code ec;

    features.path = entry.path().parent_path().wstring();
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
        features.age_in_seconds = static_cast<unsigned long>(elapsedSec);
    }
    else
    {
        features.age_in_seconds = 0;
    }

    auto perms = entry.status(ec).permissions();
    features.is_read_only = ec ? false : (perms & fs::perms::owner_write) == fs::perms::none;

    return features;
}

void print_file_features(const file_features &features)
{
    std::wcout << L"[SCAN] " << features.path << L"\\" << features.name << std::endl;
    std::wcout << L"  - Name: " << features.name << std::endl;
    std::wcout << L"  - Type: " << features.extension << std::endl;
    std::wcout << L"  - Size: " << features.file_size << L" bytes\n";
    std::wcout << L"  - Mod Time: " << features.age_in_seconds << L" seconds ago\n";
    std::wcout << L"  - Read Only: " << (features.is_read_only ? L"Yes" : L"No") << std::endl;
    std::wcout << L"-----------------------------------\n";
}

std::vector<file_features> scan_directory(const std::wstring &target_path)
{
    std::vector<file_features> collected_files;
    std::error_code ec;

    if (!fs::exists(target_path, ec))
    {
        std::wcerr << L"Error: Directory not found -> " << target_path << std::endl;
        return collected_files;
    }

    auto options = fs::directory_options::skip_permission_denied;

    for (const auto &entry : fs::recursive_directory_iterator(target_path, options, ec))
    {
        if (ec)
        {
            ec.clear();
            continue;
        }

        if (entry.is_regular_file(ec)) // skip directories, sockets, etc.
        {
            collected_files.push_back(extract_file_features(entry));
        }
    }

    return collected_files;
}