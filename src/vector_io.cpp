#include "vector_io.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>


bool saveVectors(const std::string& path, const VectorList& vectors) {
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

bool loadVectors(const std::string& path, VectorList& vectors) {
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