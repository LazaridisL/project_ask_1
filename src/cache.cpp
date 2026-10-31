#include "cache.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>


// Saves a list of vectors to a binary file
static bool saveVectors(const std::string& path, const VectorList& vectors) {
    std::uint32_t count = static_cast<std::uint32_t>(vectors.size());
    std::uint32_t dimension = vectors.empty() ? 0 : static_cast<std::uint32_t>(vectors[0].size());

    // Check for different sized vectors
    for (const Vector& vector : vectors) {
        if (vector.size() != dimension) {
            return false;
        }
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    file.write(reinterpret_cast<const char*>(&dimension), sizeof(dimension));

    for (const Vector& vector : vectors) {
        file.write(reinterpret_cast<const char*>(vector.data()), dimension * sizeof(float));
    }

    return static_cast<bool>(file);
}

// Loads a file created by saveVectors into the "vectors" variable
static bool loadVectors(const std::string& path, VectorList& vectors) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    std::uint32_t count = 0;
    std::uint32_t dimension = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    file.read(reinterpret_cast<char*>(&dimension), sizeof(dimension));
    if (!file) {
        return false;
    }

    // Catch wrong sized files
    std::uintmax_t expectedSize = 2 * sizeof(std::uint32_t) + static_cast<std::uintmax_t>(count) * dimension * sizeof(float);
    if (std::filesystem::file_size(path) != expectedSize) {
        return false;
    }

    VectorList loaded(count, Vector(dimension));
    for (Vector& vector : loaded) {
        file.read(reinterpret_cast<char*>(vector.data()), dimension * sizeof(float));
    }
    if (!file) {
        return false;
    }

    vectors = std::move(loaded);
    
    return true;
}

static bool readTextFile(const std::string& path, std::string& text) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    text = contents.str();

    return true;
}

static bool writeTextFile(const std::string& path, const std::string& text) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file << text;

    return static_cast<bool>(file);
}

std::string describeList(const std::string& title, const std::vector<std::string>& items) {
    std::string text = title + ":\n";
    for (const std::string& item : items) {
        text += item + "\n";
    }

    return text;
}

bool loadFromCache(const std::string& cacheFolder, const std::string& name, const std::string& description, VectorList& vectors) {
    if (cacheFolder.empty()) {
        return false;
    }

    const std::filesystem::path folder = cacheFolder;
    const std::string descriptionPath = (folder / (name + ".txt")).string();
    const std::string vectorsPath = (folder / (name + ".bin")).string();

    // Compare current input and the file's input
    std::string storedDescription;
    if (!readTextFile(descriptionPath, storedDescription) || (storedDescription != description)) {
        return false;
    }

    if (!loadVectors(vectorsPath, vectors)) {
        return false;
    }

    std::cerr << "Loaded " << vectors.size() << " vectors from " << vectorsPath << "\n";

    return true;
}

void saveToCache(const std::string& cacheFolder, const std::string& name, const std::string& description, const VectorList& vectors) {
    if (cacheFolder.empty()) {
        return;
    }

    const std::filesystem::path folder = cacheFolder;
    const std::string descriptionPath = (folder / (name + ".txt")).string();
    const std::string vectorsPath = (folder / (name + ".bin")).string();

    std::error_code error;
    std::filesystem::create_directories(folder, error);
    std::filesystem::remove(descriptionPath, error);    // If the program stops during saving it makes the cache not loadable

    if (!saveVectors(vectorsPath, vectors) || (!writeTextFile(descriptionPath, description))) {
        std::cerr << "Warning: could not save " << name << " to the cache folder " << cacheFolder << "\n";
    }
}