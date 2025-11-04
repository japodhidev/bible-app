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
        int ec = listResources(parse.config.resourcesRoot);
        exitCode = std::max(exitCode, ec);
    }

    if (parse.config.validate) {
        auto res = validateResources(parse.config.resourcesRoot);
        int ec = printValidationReport(res);
        exitCode = std::max(exitCode, ec);
    }

    if (!parse.config.bookName.empty() && parse.config.chapter > 0 && parse.config.verse > 0) {
        int ec = readVerse(parse.config.resourcesRoot, parse.config.bookName, parse.config.chapter, parse.config.verse);
        exitCode = std::max(exitCode, ec);
    }

    return exitCode;
}