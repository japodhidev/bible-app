#include <iostream>
#include <filesystem>

#include "../include/cli.hpp"
#include "../include/listing.hpp"
#include "../include/validation.hpp"
#include "../include/print.hpp"
#include "../include/reader.hpp"
#include "../include/config.hpp"

int main(int argc, char* argv[]) {
    auto parse = parseCommandLine(argc, argv);
    if (parse.status == CliParseStatus::Error) {
        printErr(parse.errorMessage + "\n\n");
        printOut(buildUsage(argv[0]));
        return 1;
    }
    if (parse.status == CliParseStatus::ShowHelp) {
        printOut(buildUsage(argv[0]));
        return 0;
    }

    // Load config file
    std::filesystem::path configPath = parse.config.configPath;
    if (configPath.empty()) {
        configPath = getDefaultConfigPath();
    }
    Config config = loadConfig(configPath);

    // Merge config with CLI args (CLI args override config)
    AppConfig& appConfig = parse.config;
    
    // Use config resourcesRoot only if CLI didn't specify one (check if it's the default)
    bool usingDefaultResources = (appConfig.resourcesRoot == std::filesystem::path("resources"));
    if (usingDefaultResources && !config.resourcesRoot.empty()) {
        appConfig.resourcesRoot = config.resourcesRoot;
    }
    
    // Use config default version only if CLI didn't specify one
    if (appConfig.versionId.empty() && !config.defaultVersion.empty()) {
        appConfig.versionId = config.defaultVersion;
    }

    // Validate that resources directory exists
    if (!std::filesystem::exists(appConfig.resourcesRoot) || !std::filesystem::is_directory(appConfig.resourcesRoot)) {
        printErr("Error: resources directory not found: " + appConfig.resourcesRoot.string() + "\n");
        printErr("Please specify --path or set resources_dir in config file.\n");
        return 3;
    }

    int exitCode = 0;

    if (appConfig.list) {
        const std::string defaultVersionId = "en-kjv";
        const std::string selected = appConfig.versionId.empty() ? std::string() : appConfig.versionId;
        int ec = listResources(appConfig.resourcesRoot, defaultVersionId, selected);
        exitCode = std::max(exitCode, ec);
    }

    if (appConfig.validate) {
        auto res = validateResources(appConfig.resourcesRoot);
        int ec = printValidationReport(res);
        exitCode = std::max(exitCode, ec);
    }

    if (!appConfig.reading.empty()) {
        // TODO: Parse new book + chapter + verse/s spec
        auto reading = parseReadingSpec(appConfig.reading);
        if (reading.status == CliParseStatus::Error) {
            printErr(reading.errorMessage);
        }
        appConfig.bookName = reading.bookName;
        appConfig.chapter = reading.chapter;
        appConfig.verseSpec = reading.verseSpec;

        int ec = appConfig.versionId.empty()
            ? readVersesFromSpec(appConfig.resourcesRoot, appConfig.bookName, appConfig.chapter, appConfig.verseSpec)
            : readVersesFromSpec(appConfig.resourcesRoot, appConfig.versionId, appConfig.bookName, appConfig.chapter, appConfig.verseSpec);
        exitCode = std::max(exitCode, ec);
    } else if (!appConfig.bookName.empty() && appConfig.chapter > 0) {
        // Read verses if the book name and chapter are provided.
        // Verse is now optional - if omitted, entire chapter is printed.
        int ec = appConfig.versionId.empty()
            ? readVersesFromSpec(appConfig.resourcesRoot, appConfig.bookName, appConfig.chapter, appConfig.verseSpec)
            : readVersesFromSpec(appConfig.resourcesRoot, appConfig.versionId, appConfig.bookName, appConfig.chapter, appConfig.verseSpec);
        exitCode = std::max(exitCode, ec);
    }

    return exitCode;
}