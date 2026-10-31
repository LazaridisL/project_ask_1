#include "vocabulary.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>

#include "kmeans.hpp"
#include "sift_extractor.hpp"


// Returns maxCount random descriptors or all of them if there are less than maxCount
static VectorList sampleDescriptors(const VectorList& descriptors, int maxCount, std::mt19937& generator) {
    if (descriptors.size() <= static_cast<size_t>(maxCount)) {
        return descriptors;
    }

    std::vector<int> order(descriptors.size());
    for (size_t i=0; i<order.size(); i++) {
        order[i] = static_cast<int>(i);
    }
    std::shuffle(order.begin(), order.end(), generator);

    VectorList sample;
    sample.reserve(maxCount);
    for (int i=0; i<maxCount; i++) {
        sample.push_back(descriptors[order[i]]);
    }

    return sample;
}

VectorList buildVocabulary(const std::vector<std::string>& trainingImagePaths, int K, int S, int maxIterations, unsigned int seed) {
    std::mt19937 generator(seed);
    VectorList trainingDescriptors;

    for (size_t i=0; i<trainingImagePaths.size(); i++) {
        VectorList descriptors = extractSiftDescriptors(trainingImagePaths[i]);
        VectorList sample = sampleDescriptors(descriptors, S, generator);
        trainingDescriptors.insert(trainingDescriptors.end(), sample.begin(), sample.end());

        if (((i+1) % 50 == 0) || (i+1 == trainingImagePaths.size())) {
            std::cerr << " vocabulary: " << (i + 1) << "/" << trainingImagePaths.size() << " training images, " << trainingDescriptors.size() << " descriptors\n";
        }
    }

    if (trainingDescriptors.size() < static_cast<size_t>(K)) {
        std::cerr << "Error: only " << trainingDescriptors.size() << " training descriptors. Need at least K=" << K << "\n";
        
        return {};
    }

    std::cerr << " vocabulary: running k-means with K=" << K << "\n";
    KMeansResult result = kmeans(trainingDescriptors, K, maxIterations, seed);
    std::cerr << " vocabulary: k-means finished after " << result.iterations << " iterations\n";

    return result.centroids;
}

Vector computeHistogram(const VectorList& descriptors, const VectorList& vocabulary) {
    if (descriptors.empty()) {
        return {};
    }

    Vector histogram(vocabulary.size(), 0.0f);
    for (const Vector& descriptor : descriptors) {
        int word = findNearestCentroid(descriptor, vocabulary);
        histogram[word] += 1.0f;
    }

    float descriptorCount = static_cast<float>(descriptors.size());
    for (float& value : histogram) {
        value /= descriptorCount;
    }

    return histogram;
}

VectorList computeImageVectors(const std::vector<std::string>& imagePaths, const VectorList& vocabulary, const std::string& label) {
    VectorList vectors(imagePaths.size());

    // One image per CPU core at the same time
    #pragma omp parallel for schedule(dynamic)
    for (size_t i=0; i<imagePaths.size(); i++) {
        VectorList descriptors = extractSiftDescriptors(imagePaths[i]);
        Vector histogram = computeHistogram(descriptors, vocabulary);

        if (histogram.empty()) {
            histogram.assign(vocabulary.size(), 0.0f);  // Mark it as zero vector image
        }
        vectors[i] = std::move(histogram);
    }

    std::cerr << "Finished " << label << " vectors\n";

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