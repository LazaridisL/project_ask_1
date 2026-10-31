#ifndef VOCABULARY_HPP
#define VOCABULARY_HPP

#include <string>
#include <vector>

#include "common.hpp"


// Builds the visual vocabulary by extracting the SIFT descriptors of every training
// image and keeps S random per image and clusters them with k-means into K centers.
// Returns an empty list if there are fewer than K descriptors
VectorList buildVocabulary(const std::vector<std::string>& trainingImagePaths, int K, int S, int maxIterations, unsigned int seed);

// Turns one image's descriptors into its normalized histogram
// Returns an empty vector if the image has no descriptors
Vector computeHistogram(const VectorList& descriptors, const VectorList& vocabulary);

// Computes the SIFT + histogram vector of every image
VectorList computeImageVectors(const std::vector<std::string>& imagePaths, const VectorList& vocabulary, const std::string& label);

bool isEmptyImageVector(const Vector& vector);

#endif