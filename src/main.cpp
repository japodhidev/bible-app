#include <iostream>
#include <filesystem>

#include "../include/cli.hpp"
#include "../include/listing.hpp"
#include "../include/validation.hpp"
#include "../include/print.hpp"
#include "../include/reader.hpp"

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

    int exitCode = 0;

    if (parse.config.list) {
        const std::string defaultVersionId = "en-kjv";
        const std::string selected = parse.config.versionId.empty() ? std::string() : parse.config.versionId;
        int ec = listResources(parse.config.resourcesRoot, defaultVersionId, selected);
        exitCode = std::max(exitCode, ec);
    }

    if (parse.config.validate) {
        auto res = validateResources(parse.config.resourcesRoot);
        int ec = printValidationReport(res);
        exitCode = std::max(exitCode, ec);
    }
    // Read verses if the book name and chapter are provided.
    if (!parse.config.bookName.empty() && parse.config.chapter > 0) {
        int ec = 0;
        if (!parse.config.verseRange.empty()) {
            ec = parse.config.versionId.empty()
                ? readVerses(parse.config.resourcesRoot, parse.config.bookName, parse.config.chapter, parse.config.verseRange)
                : readVerses(parse.config.resourcesRoot, parse.config.versionId, parse.config.bookName, parse.config.chapter, parse.config.verseRange);
        } else if (parse.config.verse > 0) {
            ec = parse.config.versionId.empty()
                ? readVerse(parse.config.resourcesRoot, parse.config.bookName, parse.config.chapter, parse.config.verse)
                : readVerse(parse.config.resourcesRoot, parse.config.versionId, parse.config.bookName, parse.config.chapter, parse.config.verse);
        }
        exitCode = std::max(exitCode, ec);
    }

    return exitCode;
}