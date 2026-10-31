#include "evaluation.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <utility>
#include <filesystem>

#include "dataset.hpp"
#include "metrics.hpp"


static const int relevantPerQuery = 5;

std::vector<QueryResult> runQueries(SearchMethod& method, const std::vector<ImageEntry>& database, const std::vector<ImageEntry>& queries) {
    std::vector<QueryResult> results;

    for (const ImageEntry& query : queries) {
        QueryResult result;
        result.sequenceId = query.sequenceId;

        // Only the search itself is timed
        auto start = std::chrono::steady_clock::now();
        result.ranking = method.search(query.imageVector);
        auto end = std::chrono::steady_clock::now();
        result.milliseconds = std::chrono::duration<double, std::milli>(end - start).count();

        // A result is relevant if it comes from the query's own sequence
        std::vector<bool> isRelevant;
        for (int index : result.ranking) {
            isRelevant.push_back(database[index].sequenceId == query.sequenceId);
        }

        result.recallAt5 = recall(isRelevant, 5, relevantPerQuery);
        result.recallAt10 = recall(isRelevant, 10, relevantPerQuery);
        result.averagePrecisionAt10 = averagePrecision(isRelevant, 10, relevantPerQuery);
        results.push_back(result);
    }
    return results;
}

// Averages of the four numbers over a group of results
struct Averages {
    double recallAt5 = 0.0;
    double recallAt10 = 0.0;
    double averagePrecisionAt10 = 0.0;
    double milliseconds = 0.0;
    int count = 0;
};

static Averages average(const std::vector<QueryResult>& results)
{
    Averages averages;
    for (const QueryResult& result : results) {
        averages.recallAt5 += result.recallAt5;
        averages.recallAt10 += result.recallAt10;
        averages.averagePrecisionAt10 += result.averagePrecisionAt10;
        averages.milliseconds += result.milliseconds;
        averages.count++;
    }
    if (averages.count > 0) {
        averages.recallAt5 /= averages.count;
        averages.recallAt10 /= averages.count;
        averages.averagePrecisionAt10 /= averages.count;
        averages.milliseconds /= averages.count;
    }
    return averages;
}

bool writeOutputFile(const std::string& path, const std::string& methodName, const std::vector<ImageEntry>& database, const std::vector<QueryResult>& results) {
    std::error_code error;
    const std::filesystem::path folder = std::filesystem::path(path).parent_path();
    if (!folder.empty()) {
        std::filesystem::create_directories(folder, error);
    }
    
    std::ofstream file(path);
    if (!file) {
        return false;
    }
    file << std::fixed << std::setprecision(4);

    file << methodName << "\n";
    for (const QueryResult& result : results) {
        file << "Query: " << result.sequenceId << "\n";
        for (size_t r = 0; r < result.ranking.size(); r++) {
            file << "Rank-" << (r + 1) << ": " << database[result.ranking[r]].imageId << "\n";
        }
        file << "Recall@5: " << result.recallAt5 << "\n";
        file << "Recall@10: " << result.recallAt10 << "\n";
        file << "AP@10: " << result.averagePrecisionAt10 << "\n";
    }

    Averages overall = average(results);
    file << "Mean Recall@5: " << overall.recallAt5 << "\n";
    file << "Mean Recall@10: " << overall.recallAt10 << "\n";
    file << "mAP@10: " << overall.averagePrecisionAt10 << "\n";
    file << "Average query time: " << overall.milliseconds << "\n";

    return static_cast<bool>(file);
}

void printSummary(std::ostream& out, const std::vector<QueryResult>& results) {
    std::vector<QueryResult> illumination;
    std::vector<QueryResult> viewpoint;
    int fewerThanTen = 0;

    for (const QueryResult& result : results) {
        if (isIlluminationSequence(result.sequenceId)) {
            illumination.push_back(result);
        } else {
            viewpoint.push_back(result);
        }
        if (result.ranking.size() < static_cast<size_t>(resultsPerQuery)) {
            fewerThanTen++;
        }
    }

    out << std::fixed << std::setprecision(4);
    out << "Group          Queries  Recall@5  Recall@10  mAP@10   Time(ms)\n";

    std::vector<std::pair<std::string, std::vector<QueryResult>>> groups = {
        {"all", results}, {"illumination", illumination}, {"viewpoint", viewpoint}};

    for (const auto& group : groups) {
        Averages a = average(group.second);
        out << std::left << std::setw(15) << group.first << std::right
            << std::setw(7) << a.count
            << std::setw(10) << a.recallAt5
            << std::setw(11) << a.recallAt10
            << std::setw(9) << a.averagePrecisionAt10
            << std::setw(11) << a.milliseconds << "\n";
    }
    out << "Queries with fewer than 10 results: " << fewerThanTen << "\n";
}