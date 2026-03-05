#pragma once
#include <filesystem>
#include <string>
#include <vector> 
#include <chrono>

namespace fs = std::filesystem;
namespace Chrono = std::chrono;

// Struct to hold extracted features from a file
struct file_features
{
    std::wstring path;
    std::wstring name;
    std::wstring extension;
    unsigned long file_size;
    unsigned long age_in_seconds; 
    bool is_read_only;
};

// Struct to hold a file along with its calculated score for ranking purposes
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
};

// Reference time point for age calculations (Jan 1, 2026)
// required to get a fs::file_time_type from a fixed date
static fs::file_time_type get_ref_point()
{
    const int64_t REF_DATE_UNIX = 1767225600; // Jan 1, 2026
    auto now_sys = Chrono::system_clock::now();
    auto now_file = fs::file_time_type::clock::now();
    auto ref_sys = Chrono::system_clock::from_time_t(static_cast<time_t>(REF_DATE_UNIX));

    // Duration from "Now" to "Jan 2026"
    auto diff = ref_sys - now_sys;
    return now_file + Chrono::duration_cast<fs::file_time_type::duration>(diff);
}

static const fs::file_time_type REF_FILE_TIME = get_ref_point();

// Extracts features from a single file entry
file_features extract_file_features(const fs::directory_entry &entry);

// Utility function to print the extracted features of a file in a readable format
void print_file_features(const file_features &features);

// Returns a vector containing features for ALL files in the directory
std::vector<file_features> scan_directory(const std::wstring &target_path);