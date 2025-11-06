#include "../include/reader.hpp"
#include "../include/fs_utils.hpp"
#include "../include/print.hpp"

#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

#include <rapidjson/document.h>

/**
 * Checks if the chapter file contains the given book name
 * @param chapterFile The path to the chapter file
 * @param wantedBook The book name to check for
 * @return True if the chapter file contains the given book name, false otherwise
 */
static bool chapterContainsBookName(const std::filesystem::path& chapterFile, const std::string& wantedBook) {
    if (!isRegularFile(chapterFile) || fileSize(chapterFile) == 0) return false;
    std::ifstream in(chapterFile);
    std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (json.empty()) return false;

    rapidjson::Document d;
    // Parse the JSON and check if it is a valid object. Notify the user if there is an error.
    if (d.Parse(json.c_str()).HasParseError() || !d.IsObject()) {
        printErr("invalid chapter JSON schema: " + chapterFile.string() + "\n");
        return false;
    }
    // Check if the data member is an array. Notify the user if there is an error.
    if (!d.HasMember("data") || !d["data"].IsArray()) {
        printErr("invalid chapter JSON schema[data]: " + chapterFile.string() + "\n");
        return false;
    }
    // Pre-compute lowercase wanted book name once
    std::string wBookLower = wantedBook;
    std::transform(wBookLower.begin(), wBookLower.end(), wBookLower.begin(), ::tolower);
    // Iterate through the data array and check if the book name matches.
    // Exit early on first match for efficiency.
    const auto& arr = d["data"].GetArray();
    for (auto& v : arr) {
        if (v.IsObject() && v.HasMember("book") && v["book"].IsString()) {
            // Compare case-insensitive using string view
            const char* bookStr = v["book"].GetString();
            size_t bookLen = v["book"].GetStringLength();
            // Fast case-insensitive comparison without creating string copies
            if (bookLen == wBookLower.length()) {
                bool match = true;
                for (size_t i = 0; i < bookLen; ++i) {
                    if (::tolower(static_cast<unsigned char>(bookStr[i])) != wBookLower[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) return true;
            }
        }
    }
    return false;
}

/**
 * Finds the book directory by name
 * @param booksDir The path to the books directory
 * @param bookName The name of the book to find
 * @return The path to the book directory, or empty if not found
 */
static std::filesystem::path findBookDirByName(const std::filesystem::path& booksDir, const std::string& bookName) {
    // Iterate through the books directory and check if the book name matches.
    for (const auto& bEntry : listDirectory(booksDir)) {
        if (!bEntry.is_directory()) continue;
        const auto chaptersDir = bEntry.path() / "chapters";
        if (!isDirectory(chaptersDir)) continue;
        // Check any chapter file for matching book name
        for (const auto& cEntry : listDirectory(chaptersDir)) {
            if (!cEntry.is_regular_file()) continue;
            if (chapterContainsBookName(cEntry.path(), bookName)) return bEntry.path();
        }
    }
    // Only print error message after all candidate books have been checked and not found.
    printErr("book not found: " + bookName + "\n");
    return {};
}

/**
 * Reads the verse text for the given book/chapter/verse
 * @param root The path to the resources root directory
 * @param bookName The name of the book
 * @param chapter The chapter number
 * @param verse The verse number
 * @return The exit code
 */
static int readVerseInVersion(const std::filesystem::path& versionRoot, const std::string& bookName, int chapter, int verse) {
    const auto booksDir = versionRoot / "books";
    if (!isDirectory(booksDir)) return 3;

    const auto bookDir = findBookDirByName(booksDir, bookName);
    if (bookDir.empty()) return 4;

    const auto chapterFile = bookDir / "chapters" / (std::to_string(chapter) + ".json");
    if (!isRegularFile(chapterFile) || fileSize(chapterFile) == 0) {
        printErr("chapter not found or empty: " + chapterFile.string() + "\n");
        return 3;
    }

    std::ifstream in(chapterFile);
    std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    rapidjson::Document d;
    if (d.Parse(json.c_str()).HasParseError() || !d.IsObject() || !d.HasMember("data") || !d["data"].IsArray()) {
        printErr("invalid chapter JSON schema: " + chapterFile.string() + "\n");
        return 2;
    }

    for (auto& v : d["data"].GetArray()) {
        if (!v.IsObject()) continue;
        if (v.HasMember("verse") && v["verse"].IsString() && v.HasMember("text") && v["text"].IsString()) {
            // Optimize: compare integers instead of strings
            const char* verseStr = v["verse"].GetString();
            size_t verseLen = v["verse"].GetStringLength();
            if (verseLen > 0 && std::all_of(verseStr, verseStr + verseLen, ::isdigit)) {
                int verseNum = std::stoi(std::string(verseStr, verseLen));
                if (verseNum == verse) {
                    printOut(std::string(v["text"].GetString()) + "\n");
                    return 0;
                }
            }
        }
    }

    printErr("verse not found: " + std::to_string(verse) + "\n");
    return 3;
}

// Parses a verse range string in the form "start-end" into integers. Returns true on success.
static bool parseVerseRange(const std::string& verseRange, int& startVerse, int& endVerse) {
    // expected format: digits-digits, no spaces
    if (verseRange.empty()) return false;
    auto dashPos = verseRange.find('-');
    if (dashPos == std::string::npos || dashPos == 0 || dashPos == verseRange.size() - 1) return false;
    const std::string left = verseRange.substr(0, dashPos);
    const std::string right = verseRange.substr(dashPos + 1);
    if (left.empty() || right.empty()) return false;
    if (!std::all_of(left.begin(), left.end(), ::isdigit)) return false;
    if (!std::all_of(right.begin(), right.end(), ::isdigit)) return false;
    try {
        startVerse = std::stoi(left);
        endVerse = std::stoi(right);
    } catch (...) {
        return false;
    }
    if (startVerse <= 0 || endVerse <= 0) return false;
    if (startVerse >= endVerse) return false; // first value should be smaller than the second
    return true;
}

// Helper to parse chapter file and extract verse data. Returns max verse and populates verseToText map.
// Returns max verse (0 on error), and sets verseToText if provided.
static int parseChapterFile(const std::filesystem::path& chapterFile, std::unordered_map<int, std::string>* verseToText = nullptr) {
    if (!isRegularFile(chapterFile) || fileSize(chapterFile) == 0) return 0;
    std::ifstream in(chapterFile);
    std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (json.empty()) return 0;
    rapidjson::Document d;
    if (d.Parse(json.c_str()).HasParseError() || !d.IsObject() || !d.HasMember("data") || !d["data"].IsArray()) return 0;
    
    int maxVerse = 0;
    for (auto& v : d["data"].GetArray()) {
        if (!v.IsObject()) continue;
        if (v.HasMember("verse") && v["verse"].IsString()) {
            const char* verseStr = v["verse"].GetString();
            size_t verseLen = v["verse"].GetStringLength();
            if (verseLen > 0 && std::all_of(verseStr, verseStr + verseLen, ::isdigit)) {
                int verseNum = std::stoi(std::string(verseStr, verseLen));
                maxVerse = std::max(maxVerse, verseNum);
                if (verseToText && v.HasMember("text") && v["text"].IsString()) {
                    verseToText->emplace(verseNum, std::string(v["text"].GetString()));
                }
            }
        }
    }
    return maxVerse;
}

// Reads and prints a range of verses within a single chapter for a specific version.
static int readVerseRangeInVersion(const std::filesystem::path& versionRoot, const std::string& bookName, int chapter, int startVerse, int endVerse) {
    const auto booksDir = versionRoot / "books";
    if (!isDirectory(booksDir)) return 3;

    const auto bookDir = findBookDirByName(booksDir, bookName);
    if (bookDir.empty()) return 4;

    const auto chapterFile = bookDir / "chapters" / (std::to_string(chapter) + ".json");
    // Non-existent chapter file
    if (!isRegularFile(chapterFile)) {
        printErr("chapter not found: " + std::to_string(chapter) + "\n");
        return 3;
    }
    // Empty chapter file
    if (fileSize(chapterFile) == 0) {
        printErr("chapter is empty: " + chapterFile.string() + "\n");
        return 3;
    }

    // Parse chapter file once - extract max verse and populate verseToText map simultaneously
    std::unordered_map<int, std::string> verseToText;
    const int maxVerse = parseChapterFile(chapterFile, &verseToText);
    if (maxVerse == 0) {
        printErr("invalid chapter JSON schema: " + chapterFile.string() + "\n");
        return 2;
    }
    if (endVerse > maxVerse) {
        // Validate: second value within range
        printErr("verse range exceeds available verses (max " + std::to_string(maxVerse) + ")\n");
        return 3;
    }

    bool allFound = true;
    for (int i = startVerse; i <= endVerse; ++i) {
        auto it = verseToText.find(i);
        if (it == verseToText.end()) {
            allFound = false;
            printErr("verse not found: " + std::to_string(i) + "\n");
            continue;
        }
        printOut(std::to_string(i) + " " + it->second + "\n");
    }
    return allFound ? 0 : 3;
}

/**
 * Reads the verse text for the given book/chapter/verse
 * @param root The path to the resources root directory
 * @param bookName The name of the book
 * @param chapter The chapter number
 * @param verse The verse number
 * @return The exit code
 */
int readVerse(const std::filesystem::path& root, const std::string& bookName, int chapter, int verse) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }

    // Iterate through the versions and find the requested book
    for (const auto& version : listDirectory(root)) {
        if (!version.is_directory()) continue;
        int rc = readVerseInVersion(version.path(), bookName, chapter, verse);
        if (rc == 0) return 0;
        if (rc == 2) return rc; // schema error
        // otherwise try next version if book not found/verse not present
    }
    return 4;
}

/**
 * Reads the verse text for the given book/chapter/verse in a specific version
 * @param root The path to the resources root directory
 * @param versionId The id of the version to read
 * @param bookName The name of the book to read
 * @param chapter The chapter number to read
 * @param verse The verse number to read
 * @return The exit code
 */
int readVerse(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, int verse) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }
    if (!versionId.empty()) {
        const auto versionRoot = root / versionId;
        if (!isDirectory(versionRoot)) {
            printErr("version not found: " + versionId + "\n");
            return 3;
        }
        return readVerseInVersion(versionRoot, bookName, chapter, verse);
    }
    return readVerse(root, bookName, chapter, verse);
}

/**
 * Reads multiple verses (range) for the given book/chapter
 * verseRange must be in the format "start-end" where start < end.
 */
int readVerses(const std::filesystem::path& root, const std::string& bookName, int chapter, const std::string& verseRange) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }
    int startVerse = 0, endVerse = 0;
    if (!parseVerseRange(verseRange, startVerse, endVerse)) {
        printErr("invalid verse range format, expected start-end with start < end\n");
        return 3;
    }

    // Iterate through versions until one succeeds
    for (const auto& version : listDirectory(root)) {
        if (!version.is_directory()) continue;
        int rc = readVerseRangeInVersion(version.path(), bookName, chapter, startVerse, endVerse);
        if (rc == 0) return 0;
        if (rc == 2) return rc; // schema error
        // otherwise try next version
    }
    return 4;
}

/**
 * Reads multiple verses (range) for the given book/chapter in a specific version
 */
int readVerses(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, const std::string& verseRange) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }
    int startVerse = 0, endVerse = 0;
    if (!parseVerseRange(verseRange, startVerse, endVerse)) {
        printErr("invalid verse range format, expected start-end with start < end\n");
        return 3;
    }
    if (!versionId.empty()) {
        const auto versionRoot = root / versionId;
        if (!isDirectory(versionRoot)) {
            printErr("version not found: " + versionId + "\n");
            return 3;
        }
        return readVerseRangeInVersion(versionRoot, bookName, chapter, startVerse, endVerse);
    }
    return readVerses(root, bookName, chapter, verseRange);
}