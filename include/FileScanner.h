#pragma once
#include <filesystem>
#include <string>
#include <vector> // Needed for std::vector

namespace fs = std::filesystem;

struct file_features
{
    std::wstring path;
    std::wstring name;
    std::wstring extension;
    unsigned long file_size;
    unsigned long last_write_time; // Age in seconds
    bool is_read_only;
};

// Extracts features from a single file entry
file_features extract_file_features(const fs::directory_entry& entry);

void print_file_features(const file_features& features);

// Returns a vector containing features for ALL files in the directory
std::vector<file_features> scan_directory(const std::wstring& target_path);