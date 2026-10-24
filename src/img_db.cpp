#include "img_db.hpp"
#include "sift_extractor.hpp"
#include "vocabulary.hpp"
#include "dataset.hpp"
#include <iostream>
#include <utility>
#include <cstddef>

bool make_image_vector(
    const std::string& image_path,
    const std::string& image_id,
    const std::string& sequence_id,
    const VectorList& vocabulary,
    ImageEntry& output
) {
    if (vocabulary.empty()) {
        std::cerr << "Error: vocabulary is empty.\n";
        return false;
    }

    //Get SIFT descriptors to make the histogram
    VectorList descriptors = extractSiftDescriptors(image_path);

    if (descriptors.empty()) {
        std::cerr << "No SIFT descriptors for " << image_path << '\n';
        return false;
    }

    Vector histogram = computeHistogram(descriptors, vocabulary);

    if (histogram.size() != vocabulary.size()) {
        std::cerr << "Histogram dimension error on: " << image_path << '\n';
        return false;
    }

    output.imageId = image_id;
    output.sequenceId = sequence_id;
    output.imageVector = std::move(histogram);

    return true;
}


//For each image, we create a vector
bool build_image_database(
    const std::vector<std::string>& image_paths,
    const std::vector<std::string>& image_ids,
    const std::vector<std::string>& sequence_ids,
    const VectorList& vocabulary,
    ImageDatabase& database
) {

    if (image_paths.size() != image_ids.size() ||
        image_paths.size() != sequence_ids.size()) {
        std::cerr << "Image paths, IDs, and sequence IDs has got to be of equal size\n";
        return false;
    }

    if (vocabulary.empty()) {
        std::cerr << "Empty vocabulary\n";
        return false;
    }

    ImageDatabase result;
    result.reserve(image_paths.size());
    for (std::size_t i = 0; i < image_paths.size(); ++i) {
        ImageEntry entry;
        if (make_image_vector(
                image_paths[i],
                image_ids[i],
                sequence_ids[i],
                vocabulary,
                entry)) 
        {
            result.push_back(std::move(entry));
        }
    }
    //Add entries to database
    database = std::move(result);
    return true;
}

//Test and validation splits DB
bool build_hpatches_database(
    const std::string& hpatches_folder,
    const Split& split,
    const std::string& set_name,
    const VectorList& vocabulary,
    ImageDatabase& database
) {
    const std::vector<std::string>* sequences = nullptr;

    if (set_name == "validation") {
        sequences = &split.validation;
    } else if (set_name == "test") {
        sequences = &split.test;
    } else {
        std::cerr << "Error: set_name must be validation or test.\n";
        return false;
    }

    std::vector<std::string> paths;
    std::vector<std::string> ids;
    std::vector<std::string> sequence_ids;

    paths.reserve(sequences->size() * 5);
    ids.reserve(sequences->size() * 5);
    sequence_ids.reserve(sequences->size() * 5);

    for (const std::string& sequence : *sequences) {
        for (int image_number = 2; image_number <= 6; ++image_number) {
            paths.push_back(hpatchesImagePath(hpatches_folder, sequence, image_number));
            ids.push_back(sequence + "/" + std::to_string(image_number));
            sequence_ids.push_back(sequence);
        }
    }

    if (!build_image_database(paths, ids, sequence_ids, vocabulary, database)) {
        return false;
    }

    std::cout << "HPatches " << set_name << " database: " << database.size() << " valid image vectors / " << paths.size() << " images.\n";

    return true;
}