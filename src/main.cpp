#include "sift_extractor.hpp"
#include "vector_io.hpp"

#include <iostream>
#include <string>
#include <filesystem>

//local setup caching path
const std::string caching_path = "cached_descriptors/";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./search <image> [more images]\n";
        
        return 1;
    }

    for (int i=1; i<argc; i++) {
        std::string imagePath = argv[i];
        VectorList descriptors = extractSiftDescriptors(imagePath);

        std::cout << imagePath << ": " << descriptors.size() << " descriptors";
        if (!descriptors.empty()) {
            std::cout << " of length " << descriptors[0].size();
        }

        std::cout << "\n";

        std::string cacheFile = (std::filesystem::path(caching_path) / (std::filesystem::path(imagePath).stem().string() + ".bin")).string();

        if (saveVectors(cacheFile, descriptors)) {
            std::cout << "Saved descriptors to " << cacheFile << '\n';
        } else {
            std::cout << "Could not save descriptors\n";
        }

        VectorList descriptors_loaded;

        if (loadVectors(cacheFile, descriptors_loaded)) {
            std::cout << "Loaded " << descriptors_loaded.size() << " descriptors\n";
        } else {
            std::cout << "cache error\n";
        }
    }

    return 0;
}