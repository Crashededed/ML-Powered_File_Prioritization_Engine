#include "../include/ModelScorer.h"
#include "../include/ModelWeights.h"
#include <vector>
#include <cmath>
#include <unordered_set>
#include <string>
#include <iostream>

static double compute_recency_score(unsigned long last_write_time)
{
    double secs_in_day = 60.0 * 60.0 * 24.0;
    double age_days = static_cast<double>(last_write_time) / secs_in_day;
    return 1.0 / (age_days + 1.0);
}

static bool is_valuable_ext(const std::wstring &extension)
{
    // todo: switch to json or config file
    return HIGH_VAL_EXTS.find(extension) != HIGH_VAL_EXTS.end();
}

// function iterates over trigrams of input string, hashes them, and accumulates weights
static double accumulate_hashing_weights(const std::wstring &w_input, int weight_offset)
{
    // todo: could cause loss of data
    //  Convert to UTF-8 string to match Python hashing
    std::string input(w_input.begin(), w_input.end());

    for (char &c : input)
        c = std::tolower(c); // lowercase to match Python behavior

    if (input.length() < 3)
        return 0.0;

    double hashed_weight_sum = 0.0;

    for (size_t i = 0; i <= input.length() - 3; ++i)
    {
        std::string trigram = input.substr(i, 3);
        uint32_t h = murmur3_32(trigram.c_str(), 3, 0);
        int absolute_h = std::abs(static_cast<int>(h)); // in order to match Python's hash behavior apply absolute

        int feature_index = absolute_h % 1024;
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

    double z_recency = (MODEL_WEIGHTS[0] * recency_score);
    double z_size = (MODEL_WEIGHTS[1] * size_logged);
    double z_ext = (MODEL_WEIGHTS[2] * valuable_ext);

    z += z_recency + z_size + z_ext;

    // Add Hashing Features (Weights 3-1026 for name, 1027-2050 for path)
    double z_name = accumulate_hashing_weights(features.name, 3);
    double z_path = accumulate_hashing_weights(features.path, 1027);

    z += z_name;
    z += z_path;

    double prob = 1.0 / (1.0 + std::exp(-z));

    std::wcout << L"\n--- DEBUG SCORE for: " << features.name << L" ---\n";
    std::wcout << L"  Bias: " << MODEL_BIAS << std::endl;
    std::wcout << L"  Recency (val=" << recency_score << L"): " << z_recency << std::endl;
    std::wcout << L"  Size (val=" << size_logged << L"):    " << z_size << std::endl;
    std::wcout << L"  Ext (val=" << valuable_ext << L"):     " << z_ext << std::endl;
    std::wcout << L"  Name Hash Contrib: " << z_name << std::endl;
    std::wcout << L"  Path Hash Contrib: " << z_path << std::endl;
    std::wcout << L"  FINAL LOGIT (Z):   " << z << std::endl;
    std::wcout << L"  FINAL PROBABILITY: " << prob << std::endl;
    std::wcout << L"---------------------------------------\n";

    return prob;
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
