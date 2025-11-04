#include "../include/reader.hpp"
#include "../include/fs_utils.hpp"
#include "../include/print.hpp"

#include <fstream>
#include <string>
#include <unordered_map>

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
    // Iterate through the data array and check if the book name matches.
    const auto& arr = d["data"].GetArray();
    for (auto& v : arr) {
        if (v.IsObject() && v.HasMember("book") && v["book"].IsString()) {
            if (wantedBook == v["book"].GetString()) {
                return true;
            }
        }
    }
    printErr("book not found: " + wantedBook + "\n");
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
int readVerse(const std::filesystem::path& root, const std::string& bookName, int chapter, int verse) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }

    // Iterate through the versions and find the requested book
    for (const auto& version : listDirectory(root)) {
        if (!version.is_directory()) continue;
        const auto booksDir = version.path() / "books";
        if (!isDirectory(booksDir)) continue;

        const auto bookDir = findBookDirByName(booksDir, bookName);
        if (bookDir.empty()) continue;

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
                if (std::to_string(verse) == std::string(v["verse"].GetString())) {
                    printOut(std::string(v["text"].GetString()) + "\n");
                    return 0;
                }
            }
        }

        printErr("verse not found: " + std::to_string(verse) + "\n");
        return 3;
    }

    printErr("book not found: " + bookName + "\n");
    return 3;
}


