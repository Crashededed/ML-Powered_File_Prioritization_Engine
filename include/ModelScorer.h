#pragma once
#include "FileScanner.h" 

// Include the file_features struct
double calculate_file_score(const file_features& features);

// MurmurHash3_x86_32 implementation for consistency with sklearn
uint32_t murmur3_32(const char* key, uint32_t len, uint32_t seed);