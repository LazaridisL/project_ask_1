#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <ostream>
#include <string>
#include <vector>

#include "common.hpp"
#include "database.hpp"
#include "search_method.hpp"


struct QueryResult {
    std::string sequenceId;
    std::vector<int> ranking;   // Database indices
    double recallAt5 = 0.0;
    double recallAt10 = 0.0;
    double averagePrecisionAt10 = 0.0;
    double milliseconds = 0.0;
};

// Runs every query through the search method
std::vector<QueryResult> runQueries(SearchMethod& method, const std::vector<ImageEntry>& database, const std::vector<ImageEntry>& queries);

// Writes the output file
// Returns false if the file could not be written
bool writeOutputFile(const std::string& path, const std::string& methodName, const std::vector<ImageEntry>& database, const std::vector<QueryResult>& results);

// Prints averages overall and separately for the two types of sequences
// Used in the report
void printSummary(std::ostream& out, const std::vector<QueryResult>& results);

#endif