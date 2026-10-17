#include "sift_extractor.hpp"

#include <iostream>
#include <string>


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
    }

    return 0;
}