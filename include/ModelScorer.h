#pragma once
#include "FileScanner.h" 
#include <vector>
#include <unordered_set>
#include <string>

// Context struct to hold model parameters and weights
struct ModelContext {
    std::wstring target; 
    const std::vector<double>& weights;
    double bias;
    const std::unordered_set<std::wstring>& high_val_exts;
    const std::unordered_set<std::wstring>& junk_exts;
};

// Include the file_features struct
double calculate_file_score(const file_features& features, ModelContext context, bool debug_mode = false);

// MurmurHash3_x86_32 implementation for consistency with sklearn
uint32_t murmur3_32(const char* key, uint32_t len, uint32_t seed);