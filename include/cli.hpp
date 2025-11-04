#pragma once

#include <filesystem>
#include <string>

struct AppConfig {
    std::filesystem::path resourcesRoot {"resources"};
    bool list {false};
    bool validate {false};
    bool help {false};
    // Reading options
    std::string bookName {};
    int chapter {0};
    int verse {0};
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

CliParseResult parseCommandLine(int argc, char* argv[]);

std::string buildUsage(const char* argv0);

#pragma once

#include <filesystem>
#include <string>

struct AppConfig {
    std::filesystem::path resourcesRoot {"resources"};
    bool list {false};
    bool validate {false};
    bool help {false};
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

CliParseResult parseCommandLine(int argc, char* argv[]);

std::string buildUsage(const char* argv0);


