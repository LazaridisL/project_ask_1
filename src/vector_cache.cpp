#include "vector_cache.hpp"

#include <chrono>
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <sstream>

#include "sift_extractor.hpp"
#include "vector_io.hpp"
#include "vocabulary.hpp"


static const int maxKMeansIterations = 50;

std::string cacheFingerprint(const std::vector<std::string>& texts) {
    std::uint64_t hash = 14695981039346656037ULL;
    const std::uint64_t prime = 1099511628211ULL;

    for (const std::string& text : texts) {
        for (char character : text) {
            hash ^= static_cast<unsigned char>(character);
            hash *= prime;
        }
        // Separator
        hash ^= 0xFF;
        hash *= prime;
    }

    std::ostringstream hex;
    hex << std::hex << std::setw(16) << std::setfill('0') << hash;

    return hex.str();
}

VectorList loadOrBuildVocabulary(const std::vector<std::string>& trainingImagePaths,
                                 int K, int S, unsigned int seed,
                                 const std::string& cacheFile) {

    VectorList vocabulary;
    if ((!cacheFile.empty()) && (loadVectors(cacheFile, vocabulary))
        && (vocabulary.size() == static_cast<size_t>(K))) {
        std::cerr << "Loaded vocabulary from " << cacheFile << "\n";
        return vocabulary;
    }

    std::cerr << "Building vocabulary from " << trainingImagePaths.size() << " training images\n";
    vocabulary = buildVocabulary(trainingImagePaths, K, S, maxKMeansIterations, seed);
    if (vocabulary.empty()) {
        return {};
    }

    if ((!cacheFile.empty()) && (!saveVectors(cacheFile, vocabulary))) {
        std::cerr << "Warning: could not save vocabulary to " << cacheFile << "\n";
    }
    return vocabulary;
} 

VectorList loadOrComputeImageVectors(const std::vector<std::string>& imagePaths,
                                     const VectorList& vocabulary,
                                     const std::string& cacheFile,
                                     const std::string& label) {

    VectorList vectors;
    if ((!cacheFile.empty()) && (loadVectors(cacheFile, vectors)) && (vectors.size() == imagePaths.size())
        && (vectors.empty() || (vectors[0].size() == vocabulary.size()))) {
        std::cerr << "Loaded " << vectors.size() << " " << label << " vectors\n";
        return vectors;
    }

    std::cerr << "Computing " << label << " vectors for " << imagePaths.size() << " images\n";
    vectors.assign(imagePaths.size(), Vector());
    size_t done = 0;
    auto start = std::chrono::steady_clock::now();

    // One image per CPU core is processed at the same time
    #pragma omp parallel for schedule(dynamic)
    for (size_t i=0; i<imagePaths.size(); i++) {
        VectorList descriptors = extractSiftDescriptors(imagePaths[i]);
        Vector histogram = computeHistogram(descriptors, vocabulary);

        if (histogram.empty()) {
            histogram.assign(vocabulary.size(), 0.0f);   // all-zero placeholder
        }
        vectors[i] = std::move(histogram);

        // Only one thread at a time may update the counter and print
        #pragma omp critical
        {
            done++;
            if (done % 500 == 0 || done == imagePaths.size()) {
                double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                double secondsLeft = seconds / done * (imagePaths.size() - done);
                std::cerr << "  " << label << ": " << done << "/" << imagePaths.size()
                          << " images, " << static_cast<int>(seconds / 60) << " min so far, about "
                          << static_cast<int>(secondsLeft / 60) << " min left\n";
            }
        }
    }

    if (!cacheFile.empty() && !saveVectors(cacheFile, vectors)) {
        std::cerr << "Warning: could not save " << label << " vectors to " << cacheFile << "\n";
    }
    return vectors;
}

bool isEmptyImageVector(const Vector& vector) {
    for (float value : vector) {
        if (value != 0.0f) {
            return false;
        }
    }

    return true;
}