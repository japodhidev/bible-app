#include "../include/validation.hpp"
#include "../include/fs_utils.hpp"
#include "../include/print.hpp"

#include <algorithm>
#include <set>

/**
 * Adds an issue to the validation result
 * @param r The validation result
 * @param s The severity of the issue
 * @param p The path to the issue
 * @param m The message of the issue
 */
static void addIssue(ValidationResult& r, Severity s, const std::filesystem::path& p, const std::string& m) {
    r.issues.push_back(ValidationIssue{ s, p, m });
    if (s == Severity::Error) r.ok = false;
}

/**
 * Validates the resources directory structure
 * @param root The path to the resources root directory
 * @return The validation result
 */
ValidationResult validateResources(const std::filesystem::path& root) {
    ValidationResult result{};

    if (!isDirectory(root)) {
        addIssue(result, Severity::Error, root, "resources root is missing or not a directory");
        return result;
    }

    for (const auto& tEntry : listDirectory(root)) {
        if (!tEntry.is_directory()) continue;
        const auto translationDir = tEntry.path();
        const auto translationId = translationDir.filename().string();

        // Expect <translationId>.json
        const auto metaFile = translationDir / (translationId + ".json");
        if (!isRegularFile(metaFile)) {
            addIssue(result, Severity::Error, metaFile, "missing translation metadata JSON file");
        }

        // Expect books/
        const auto booksDir = translationDir / "books";
        if (!isDirectory(booksDir)) {
            addIssue(result, Severity::Error, booksDir, "missing books directory");
            continue;
        }

        for (const auto& bEntry : listDirectory(booksDir)) {
            if (!bEntry.is_directory()) continue;
            const auto bookDir = bEntry.path();
            const auto chaptersDir = bookDir / "chapters";

            if (!isDirectory(chaptersDir)) {
                addIssue(result, Severity::Error, chaptersDir, "missing chapters directory");
                continue;
            }

            std::set<int> chapterNumbers;
            for (const auto& cEntry : listDirectory(chaptersDir)) {
                if (!cEntry.is_regular_file()) continue;
                const auto fname = cEntry.path().filename().string();
                if (!matchesNumericJson(fname)) {
                    addIssue(result, Severity::Error, cEntry.path(), "chapter file name must be N.json where N>=1");
                    continue;
                }
                // Ensure file not empty
                if (fileSize(cEntry.path()) == 0) {
                    addIssue(result, Severity::Error, cEntry.path(), "chapter JSON file is empty");
                    continue;
                }
                // extract number
                const auto stem = cEntry.path().stem().string();
                try {
                    int n = std::stoi(stem);
                    if (n <= 0) {
                        addIssue(result, Severity::Error, cEntry.path(), "chapter number must be positive");
                    } else {
                        chapterNumbers.insert(n);
                    }
                } catch (...) {
                    addIssue(result, Severity::Error, cEntry.path(), "invalid chapter number");
                }
            }
            // Accomodate for non-contiguous chapter numbers but notify the user rather than fail
            if (!chapterNumbers.empty()) {
                // Check contiguous 1..N
                int expected = 1;
                for (int n : chapterNumbers) {
                    if (n != expected) {
                        addIssue(result, Severity::Error, chaptersDir, "chapter numbers must be contiguous starting at 1");
                        continue;
                    }
                    ++expected;
                }
            }
        }
    }

    return result;
}

/**
 * Prints the validation report
 * @param result The validation result
 * @return The exit code
 */
int printValidationReport(const ValidationResult& result) {
    for (const auto& issue : result.issues) {
        const char* sev = (issue.severity == Severity::Error) ? "ERROR" : "WARN";
        printErr(std::string(sev) + ": " + issue.path.string() + ": " + issue.message + "\n");
    }
    return result.ok ? 0 : 2;
}


