#include "dataset.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>


namespace fs = std::filesystem;

bool readSplit(const std::string& path, Split& split) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: could not open split file " << path << "\n";
        return false;
    }

    Split result;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;

        std::istringstream words(line);
        std::string sequence;
        std::string setName;

        if(!(words >> sequence)) {
            continue;
        }
        if (!(words >> setName)) {
            std::cerr << "Error: line " << lineNumber << " of " << path << " has no set name\n";
            return false;
        }

        if (setName == "train") {
            result.train.push_back(sequence);
        } else if (setName == "validation") {
            result.validation.push_back(sequence);
        } else if (setName == "test") {
            result.test.push_back(sequence);
        } else {
            std::cerr << "Error: line " << lineNumber << " of " << path << " has unknown set\"" << setName << "\"\n";
            return false;
        }

    }

    split = std::move(result);

    return true;
}

std::string hpatchesImagePath(const std::string& hpatchesFolder, const std::string& sequence, int imageNumber) {
    fs::path path = fs::path(hpatchesFolder) / sequence / (std::to_string(imageNumber) + ".ppm");

    return path.string();
}

bool isIlluminationSequence(const std::string& sequence) {
    return sequence.rfind("i_", 0) == 0;
}

// Reads the number from an image name (e.g. "im27.jph")
// Returns false if the name is not in that format
static bool parseMirflickrNumber(const std::string& fileName, int& number) {
    const std::string prefix = "im";
    const std::string suffix = ".jpg";

    if (fileName.size() <= prefix.size() + suffix.size()) {
        return false;   
    }
    if (fileName.rfind(prefix, 0) != 0) {
        return false;
    }
    if (fileName.compare(fileName.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return false;
    }

    std::string digits = fileName.substr(prefix.size(), fileName.size() - prefix.size() - suffix.size());
    for (char character : digits) {
        if (!std::isdigit(static_cast<unsigned char>(character))) {
            return false;
        }
    }

    number = std::stoi(digits);
    
    return true;
}

std::vector<std::string> listMirflickrImages(const std::string& mirflickrFolder) {
    std::vector<std::pair<int, std::string>> numberedImages;

    std::error_code error;
    for (const fs::directory_entry& entry : fs::directory_iterator(mirflickrFolder, error)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        int number = 0;
        if (parseMirflickrNumber(entry.path().filename().string(), number)) {
            numberedImages.push_back({number, entry.path().string()});
        }
    }

    if (error) {
        std::cerr << "Error: could not read folder " << mirflickrFolder << "\n";
        return {};
    }

    std::sort(numberedImages.begin(), numberedImages.end());

    std::vector<std::string> paths;
    paths.reserve(numberedImages.size());
    for (const std::pair<int, std::string>& image : numberedImages) {
        paths.push_back(image.second);
    }

    return paths;
}