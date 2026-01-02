#include "../include/ModelScorer.h"
#include "../include/ModelWeights.h"
#include <vector>
#include <cmath>
#include <unordered_set>
#include <string>
#include <iostream>

static double compute_recency_score(unsigned long age_in_seconds)
{
    //todo: switch to system clock if issues arise
    // Using a half-life of 90 days for recency scoring
    double secs_in_day = 60.0 * 60.0 * 24.0;
    return 1.0 / (static_cast<double>(age_in_seconds) / (90.0 * secs_in_day) + 1.0);
}

static int is_valuable_ext(const std::wstring &extension, const std::unordered_set<std::wstring> &high_val_exts)
{
    return high_val_exts.find(extension) != high_val_exts.end() ? 1 : 0;
}

static int is_junk_ext(const std::wstring &extension, const std::unordered_set<std::wstring> &junk_exts)
{
    return junk_exts.find(extension) != junk_exts.end() ? 1 : 0;
}

// function iterates over trigrams of input string, hashes them, and accumulates weights
static double accumulate_hashing_weights(const std::wstring &w_input, const std::vector<double> &model_weights, int weight_offset)
{
    // todo: could cause loss of data
    //  Convert to UTF-8 string to match Python hashing
    std::string input(w_input.begin(), w_input.end());

    for (char &c : input)
        c = std::tolower(c); // lowercase to match Python behavior

    if (input.length() < 4)
        return 0.0;

    double hashed_weight_sum = 0.0;

    for (size_t i = 0; i <= input.length() - 4; ++i)
    {
        std::string trigram = input.substr(i, 4);
        uint32_t h = murmur3_32(trigram.c_str(), 4, 0);
        int absolute_h = std::abs(static_cast<int>(h)); // in order to match Python's hash behavior apply absolute

        int feature_index = absolute_h % 1024;
        hashed_weight_sum += model_weights[weight_offset + feature_index];
    }

    return hashed_weight_sum;
}

double calculate_file_score(const file_features &features, ModelContext context, bool debug_mode)
{
    double z = context.bias;

    double recency_score = compute_recency_score(features.age_in_seconds);
    double size_logged = std::log1p(features.file_size);
    double valuable_ext = is_valuable_ext(features.extension, context.high_val_exts);
    double junk_ext = is_junk_ext(features.extension, context.junk_exts);
    double name_len = static_cast<double>(features.name.length());
    double path_len = static_cast<double>(features.path.length());
    double path_depth = static_cast<double>(std::count(features.path.begin(), features.path.end(), L'\\') +
                                            std::count(features.path.begin(), features.path.end(), L'/'));

    double z_recency = (context.weights[0] * recency_score);
    double z_size = (context.weights[1] * size_logged);
    double z_ext = (context.weights[2] * valuable_ext);
    double z_junk_ext = (context.weights[3] * junk_ext);
    double z_name_len = (context.weights[4] * name_len);
    double z_path_len = (context.weights[5] * path_len);
    double z_path_depth = (context.weights[6] * path_depth);
    z += z_recency + z_size + z_ext + z_junk_ext + z_name_len + z_path_len + z_path_depth;

    // Add Hashing Features (Weights 3-1026 for name, 1027-2050 for path)
    double z_name = accumulate_hashing_weights(features.name, context.weights, 7);
    double z_path = accumulate_hashing_weights(features.path, context.weights, 1031);
    z += z_name;
    z += z_path;

    double prob = 1.0 / (1.0 + std::exp(-z));

    if (debug_mode)
    {
        std::wcout << L"\n--- DEBUG SCORE for " << context.target << L": - " << features.name << L" ---\n";
        std::wcout << L"  Bias: " << context.bias << std::endl;
        std::wcout << L"  Recency (val=" << recency_score << L"): " << z_recency << std::endl;
        std::wcout << L"  Size (val=" << size_logged << L"):    " << z_size << std::endl;
        std::wcout << L"  Name Length (val=" << name_len << L"): " << z_name_len << std::endl;
        std::wcout << L"  Path Length (val=" << path_len << L"): " << z_path_len << std::endl;
        std::wcout << L"  Path Depth (val=" << path_depth << L"): " << z_path_depth << std::endl;
        std::wcout << L"  Ext (val=" << valuable_ext << L"):     " << z_ext << std::endl;
        std::wcout << L"  Junk Ext (val=" << junk_ext << L"):  " << z_junk_ext << std::endl;
        std::wcout << L"  Name Hash Contrib: " << z_name << std::endl;
        std::wcout << L"  Path Hash Contrib: " << z_path << std::endl;
        std::wcout << L"  FINAL LOGIT (Z):   " << z << std::endl;
        std::wcout << L"  FINAL PROBABILITY: " << prob << std::endl;
        std::wcout << L"---------------------------------------\n";
    }
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
