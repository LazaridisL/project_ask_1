#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "not enough args passed: make run IMG_PATH=path/to/img.png \n";
        return 1;
    }
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);

    if (img.empty()) {
        std::cerr << "Could not load image: " << argv[1] << '\n';
        return 1;
    }

    auto sift = cv::SIFT::create();

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat sift_descriptors;

    sift->detectAndCompute(
        img, cv::noArray(), keypoints, sift_descriptors
    );

    std::cout << "img path: " << argv[1] << '\n';
    std::cout << "keypoints: " << keypoints.size() << '\n';
    std::cout << "descriptors: " << sift_descriptors.rows << " x " << sift_descriptors.cols << '\n';

    return 0;
}