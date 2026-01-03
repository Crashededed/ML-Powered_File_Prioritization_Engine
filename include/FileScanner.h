#pragma once
#include <filesystem>
#include <string>
#include <vector> 
#include <chrono>

namespace fs = std::filesystem;
namespace Chrono = std::chrono;

struct file_features
{
    std::wstring path;
    std::wstring name;
    std::wstring extension;
    unsigned long file_size;
    unsigned long age_in_seconds; 
    bool is_read_only;
};

struct scoredFile
{
    file_features file;
    double score;

    // Greater-than operator 
    bool operator>(const scoredFile& other) const {
        return score > other.score;
    }
    
    // Less-than operator 
    bool operator<(const scoredFile& other) const {
        return score < other.score;
    }

    //todo: add equality operator if needed
};

// Reference time point for age calculations (Jan 1, 2026)
// required to get a fs::file_time_type from a fixed date
static fs::file_time_type get_ref_point()
{
    const int64_t REF_DATE_UNIX = 1767225600; // Jan 1, 2026
    auto now_sys = Chrono::system_clock::now();
    auto now_file = fs::file_time_type::clock::now();
    auto ref_sys = Chrono::system_clock::from_time_t(static_cast<time_t>(REF_DATE_UNIX));

    // Distance from "Now" to "Jan 2026"
    auto diff = ref_sys - now_sys;
    return now_file + Chrono::duration_cast<fs::file_time_type::duration>(diff);
}

static const fs::file_time_type REF_FILE_TIME = get_ref_point();

// Extracts features from a single file entry
file_features extract_file_features(const fs::directory_entry &entry);

void print_file_features(const file_features &features);

// Returns a vector containing features for ALL files in the directory
std::vector<file_features> scan_directory(const std::wstring &target_path);