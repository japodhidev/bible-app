#pragma once

#include <filesystem>
#include <string>
#include <regex>
#include <vector>

struct AppConfig {
    std::filesystem::path resourcesRoot {"resources"};
    bool list {false};
    bool validate {false};
    bool help {false};
    // Reading options
    std::string bookName {};
    int chapter {0};
    std::string verseSpec {};  // Can be: single number, range "a-b", or comma-separated list "23, 27, 34-40"
    std::string reading {}; // Supported formats: Joshua 1:9, Joshua 1:9-13, Joshua 1:9-13, 14, 20
    // Version selection
    std::string versionId {};
    // Config file
    std::filesystem::path configPath {};
};

enum class CliParseStatus {
    Ok,
    ShowHelp,
    Error
};

struct CliParseResult {
    CliParseStatus status {CliParseStatus::Ok};
    AppConfig config {};
    std::string errorMessage {};
};

struct ReadingSpec {
    std::string bookName {};
    int chapter {};
    std::string verseSpec {};
    CliParseStatus status {CliParseStatus::Ok};
    std::string errorMessage {};
};

CliParseResult parseCommandLine(int argc, char* argv[]);

std::string buildUsage(const char* argv0);

ReadingSpec parseReadingSpec(const std::string& input);
