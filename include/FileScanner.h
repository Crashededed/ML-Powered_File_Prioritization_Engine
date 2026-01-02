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

// Extracts features from a single file entry
file_features extract_file_features(const fs::directory_entry &entry);

void print_file_features(const file_features &features);

// Returns a vector containing features for ALL files in the directory
std::vector<file_features> scan_directory(const std::wstring &target_path);