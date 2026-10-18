#include "sift_extractor.hpp"

#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <cstdint>
#include <fstream>
#include <string>


VectorList extractSiftDescriptors(const std::string& imagePath) {
    VectorList result;

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Warning: could not read image: " << imagePath << "\n";

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

//save descriptors to yml file
bool save_descriptors(const std::string& path, const VectorList& descriptors) {
    constexpr std::uint32_t cols = 128;

    if (descriptors.empty()) {
        return false;
    }

    for (const auto& descriptor : descriptors) {
        if (descriptor.size() != cols) {
            return false;
        }
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }

    const std::uint32_t rows =
        static_cast<std::uint32_t>(descriptors.size());

    out.write(
        reinterpret_cast<const char*>(&rows),
        sizeof(rows)
    );

    out.write(
        reinterpret_cast<const char*>(&cols),
        sizeof(cols)
    );

    for (const auto& descriptor : descriptors) {
        out.write(
            reinterpret_cast<const char*>(descriptor.data()),
            static_cast<std::streamsize>(
                descriptor.size() * sizeof(float)
            )
        );
    }

    return static_cast<bool>(out);
}

//load the sift descriptors from the cache file
bool load_descriptors(const std::string& path, VectorList& descriptors) {
    //descriptors.clear();

    constexpr std::uint32_t expected_columns = 128;
    constexpr std::uint64_t max_descriptors = 10'000'000;
    VectorList loaded(rows);

    std::ifstream desc_load(path, std::ios::binary);
    if (!desc_load) {
        return false;
    }

    std::uint32_t rows = 0;
    std::uint32_t cols = 0;

    desc_load.read(
        reinterpret_cast<char*>(&rows),
        sizeof(rows)
    );

    desc_load.read(
        reinterpret_cast<char*>(&cols),
        sizeof(cols)
    );

    if (!desc_load || cols != 128) {
        return false;
    }

    if (rows > max_descriptors ||
        rows > std::numeric_limits<std::size_t>::max()
                   / expected_columns) {
        return false;
    }

    for (auto& descriptor : loaded) {
        descriptor.resize(cols);

        desc_load.read(
            reinterpret_cast<char*>(descriptor.data()),
            static_cast<std::streamsize>(
                descriptor.size() * sizeof(float)
            )
        );

        if (!desc_load) {
            return false;
        }
    }

    if (desc_load.peek() != std::char_traits<char>::eof()) {
        return false;
    }

    descriptors = std::move(loaded);
    return true;
}