#ifndef VOCABULARY_HPP
#define VOCABULARY_HPP

#include <random>
#include <string>
#include <vector>

#include "common.hpp"


// Returns maxCount unique random descriptors
// If there are fewer than maxCount, all of them are returned
VectorList sampleDescriptors(const VectorList& descriptors, int maxCount, std::mt19937& generator);

// Builds the visual vocabulary by extracting the SIFT descriptors of every training
// image and keeps S random per image and clusters them with k-means into K centers.
// Returns an empty list if there are fewer than K descriptors
VectorList buildVocabulary(const std::vector<std::string>& trainingImagePaths, int K, int S, int maxIterations, unsigned int seed);

// Turns one image's descriptors into its normalized histogram
// Returns an empty vector if the image has no descriptors
Vector computeHistogram(const VectorList& descriptors, const VectorList& vocabulary);

#endif