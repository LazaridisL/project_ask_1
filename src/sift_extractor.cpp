#include "sift_extractor.hpp"

#include <iostream>

#include <opencv2/opencv.hpp>


VectorList extractSiftDescriptors(const std::string& imagePath) {
    VectorList result;

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Error: could not read image: " << imagePath << "\n";

        return result;
    }

    // Use static to create only once and then be reused
    static auto sift = cv::SIFT::create();

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    sift->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

    result.reserve(descriptors.rows);
    for (int row=0; row<descriptors.rows; row++) {
        const float* rowStart = descriptors.ptr<float>(row);
        result.emplace_back(rowStart, rowStart+descriptors.cols);
    }

    return result;
}