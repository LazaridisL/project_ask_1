
#ifndef IMAGE_DATABASE_HPP
#define IMAGE_DATABASE_HPP

#include <vector>
#include <string>
#include "common.hpp"
#include "dataset.hpp"

using ImageDatabase = std::vector<ImageEntry>;

bool make_image_vector(
    const std::string& image_path,
    const std::string& image_id,
    const std::string& sequence_id,
    const VectorList& vocabulary,
    ImageEntry& output
);

bool build_image_database(
    const std::vector<std::string>& image_paths,
    const std::vector<std::string>& image_ids,
    const std::vector<std::string>& sequence_ids,
    const VectorList& vocabulary,
    ImageDatabase& database
);

bool build_hpatches_database(
    const std::string& hpatches_folder,
    const Split& split,
    const std::string& set_name,
    const VectorList& vocabulary,
    ImageDatabase& database
);

#endif
