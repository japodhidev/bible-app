#include "../include/cli.hpp"

#include <sstream>

static bool isFlag(const std::string& token) {
    return !token.empty() && token[0] == '-';
}

std::string buildUsage(const char* argv0) {
    std::ostringstream oss;
    oss << "Usage: " << argv0 << " [OPTIONS]\n\n"
        << "Options:\n"
        << "  -l, --list             List available versions (with version & description)\n"
        << "  -v, --validate         Validate resources directory structure\n"
        << "  -p, --path <dir>       Set resources root directory (default: resources)\n"
        << "      --version <id>     Select version id (default: en-kjv)\n"
        << "      --book <name>      Select book name to read\n"
        << "      --chapter <n>      Select chapter number\n"
        << "      --verse <n>        Select verse number\n"
        << "  -h, --help             Show this help message\n";
    return oss.str();
}

/**
 * Parses the command line arguments and returns the parsed result
 * @param argc The number of arguments
 * @param argv The arguments
 * @return The parsed result
 */
CliParseResult parseCommandLine(int argc, char* argv[]) {
    CliParseResult result{};
    AppConfig config{};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            result.status = CliParseStatus::ShowHelp;
            result.config = config;
            return result;
        } else if (arg == "-l" || arg == "--list") {
            config.list = true;
        } else if (arg == "-v" || arg == "--validate") {
            config.validate = true;
        } else if (arg == "-p" || arg == "--path") {
            if (i + 1 >= argc) {
                result.status = CliParseStatus::Error;
                result.errorMessage = "Missing value for option: " + arg;
                return result;
            }
            std::string value = argv[++i];
            if (isFlag(value)) {
                result.status = CliParseStatus::Error;
                result.errorMessage = "Option requires a value: " + arg;
                return result;
            }
            config.resourcesRoot = value;
        } else if (arg == "--version") {
            if (i + 1 >= argc) { result.status = CliParseStatus::Error; result.errorMessage = "Missing value for --version"; return result; }
            std::string value = argv[++i];
            if (isFlag(value)) { result.status = CliParseStatus::Error; result.errorMessage = "--version requires a value"; return result; }
            config.versionId = value;
        } else if (arg == "--book") {
            if (i + 1 >= argc) { result.status = CliParseStatus::Error; result.errorMessage = "Missing value for --book"; return result; }
            std::string value = argv[++i];
            if (isFlag(value)) { result.status = CliParseStatus::Error; result.errorMessage = "--book requires a value"; return result; }
            config.bookName = value;
        } else if (arg == "--chapter") {
            if (i + 1 >= argc) { result.status = CliParseStatus::Error; result.errorMessage = "Missing value for --chapter"; return result; }
            std::string value = argv[++i];
            if (isFlag(value)) { result.status = CliParseStatus::Error; result.errorMessage = "--chapter requires a value"; return result; }
            try { config.chapter = std::stoi(value); } catch (...) { result.status = CliParseStatus::Error; result.errorMessage = "--chapter must be an integer"; return result; }
        } else if (arg == "--verse") {
            if (i + 1 >= argc) { result.status = CliParseStatus::Error; result.errorMessage = "Missing value for --verse"; return result; }
            std::string value = argv[++i];
            if (isFlag(value)) { result.status = CliParseStatus::Error; result.errorMessage = "--verse requires a value"; return result; }
            try { config.verse = std::stoi(value); } catch (...) { result.status = CliParseStatus::Error; result.errorMessage = "--verse must be an integer"; return result; }
        } else {
            result.status = CliParseStatus::Error;
            result.errorMessage = "Unknown option: " + arg;
            return result;
        }
    }

    if (!config.list && !config.validate && (config.bookName.empty() || config.chapter <= 0 || config.verse <= 0)) {
        // If no action flags, default to help to be explicit.
        result.status = CliParseStatus::ShowHelp;
        result.config = config;
        return result;
    }

    result.status = CliParseStatus::Ok;
    result.config = config;
    return result;
}


