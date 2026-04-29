#include "../include/reader.hpp"
#include "../include/fs_utils.hpp"
#include "../include/print.hpp"
#include "../include/format_utils.hpp"

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
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
 * Expands short book name abbreviations to full book names
 * @param shortName The short name or abbreviation
 * @return The full book name, or the original string if no mapping found
 */
static std::string expandBookName(const std::string& shortName) {
    // Create a static map of short names to full names
    static const std::unordered_map<std::string, std::string> shortNameMap = {
        // Old Testament
        {"Gen", "Genesis"},
        {"Ex", "Exodus"},
        {"Exod", "Exodus"},
        {"Lev", "Leviticus"},
        {"Num", "Numbers"},
        {"Deut", "Deuteronomy"},
        {"Josh", "Joshua"},
        {"Judg", "Judges"},
        {"Ruth", "Ruth"},
        {"1 Sam", "1 Samuel"},
        {"1Sam", "1 Samuel"},
        {"1Samuel", "1 Samuel"},
        {"2 Sam", "2 Samuel"},
        {"2Sam", "2 Samuel"},
        {"2Samuel", "2 Samuel"},
        {"1 Kgs", "1 Kings"},
        {"1Kgs", "1 Kings"},
        {"1Kings", "1 Kings"},
        {"2 Kgs", "2 Kings"},
        {"2Kgs", "2 Kings"},
        {"2Kings", "2 Kings"},
        {"1 Chr", "1 Chronicles"},
        {"1Chr", "1 Chronicles"},
        {"1Chronicles", "1 Chronicles"},
        {"2 Chr", "2 Chronicles"},
        {"2Chr", "2 Chronicles"},
        {"2Chronicles", "2 Chronicles"},
        {"Ezra", "Ezra"},
        {"Neh", "Nehemiah"},
        {"Esth", "Esther"},
        {"Esther", "Esther"},
        {"Job", "Job"},
        {"Ps", "Psalms"},
        {"Pss", "Psalms"},
        {"Prov", "Proverbs"},
        {"Eccl", "Ecclesiastes"},
        {"Song", "Song of Solomon"},
        {"Song of Sol", "Song of Solomon"},
        {"Isa", "Isaiah"},
        {"Jer", "Jeremiah"},
        {"Lam", "Lamentations"},
        {"Ezek", "Ezekiel"},
        {"Dan", "Daniel"},
        {"Hos", "Hosea"},
        {"Joel", "Joel"},
        {"Amos", "Amos"},
        {"Obad", "Obadiah"},
        {"Jonah", "Jonah"},
        {"Mic", "Micah"},
        {"Nah", "Nahum"},
        {"Hab", "Habakkuk"},
        {"Zeph", "Zephaniah"},
        {"Hag", "Haggai"},
        {"Zech", "Zechariah"},
        {"Mal", "Malachi"},
        // New Testament
        {"Matt", "Matthew"},
        {"Mark", "Mark"},
        {"Luke", "Luke"},
        {"John", "John"},
        {"Acts", "Acts"},
        {"Rom", "Romans"},
        {"1 Cor", "1 Corinthians"},
        {"1Cor", "1 Corinthians"},
        {"1Corinthians", "1 Corinthians"},
        {"2 Cor", "2 Corinthians"},
        {"2Cor", "2 Corinthians"},
        {"2Corinthians", "2 Corinthians"},
        {"Gal", "Galatians"},
        {"Eph", "Ephesians"},
        {"Phil", "Philippians"},
        {"Col", "Colossians"},
        {"1 Thess", "1 Thessalonians"},
        {"1Thess", "1 Thessalonians"},
        {"1Thessalonians", "1 Thessalonians"},
        {"2 Thess", "2 Thessalonians"},
        {"2Thess", "2 Thessalonians"},
        {"2Thessalonians", "2 Thessalonians"},
        {"1 Tim", "1 Timothy"},
        {"1Tim", "1 Timothy"},
        {"1Timothy", "1 Timothy"},
        {"2 Tim", "2 Timothy"},
        {"2Tim", "2 Timothy"},
        {"2Timothy", "2 Timothy"},
        {"Titus", "Titus"},
        {"Phlm", "Philemon"},
        {"Philem", "Philemon"},
        {"Heb", "Hebrews"},
        {"Jas", "James"},
        {"James", "James"},
        {"1 Pet", "1 Peter"},
        {"1Pet", "1 Peter"},
        {"1Peter", "1 Peter"},
        {"2 Pet", "2 Peter"},
        {"2Pet", "2 Peter"},
        {"2Peter", "2 Peter"},
        {"1 John", "1 John"},
        {"1John", "1 John"},
        {"2 John", "2 John"},
        {"2John", "2 John"},
        {"3 John", "3 John"},
        {"3John", "3 John"},
        {"Jude", "Jude"},
        {"Rev", "Revelation"},
        {"Revelation", "Revelation"}
    };
    
    // Normalize input: trim and handle case-insensitive lookup
    std::string normalized = shortName;
    // Trim leading whitespace
    size_t first = normalized.find_first_not_of(" \t");
    if (first != std::string::npos) {
        normalized.erase(0, first);
    } else {
        normalized.clear();
    }
    // Trim trailing whitespace
    if (!normalized.empty()) {
        size_t last = normalized.find_last_not_of(" \t");
        if (last != std::string::npos) {
            normalized.erase(last + 1);
        } else {
            normalized.clear();
        }
    }
    
    // Try exact match first (case-sensitive)
    auto it = shortNameMap.find(normalized);
    if (it != shortNameMap.end()) {
        return it->second;
    }
    
    // Try case-insensitive match for short names (keys)
    std::string lowerNormalized = normalized;
    std::transform(lowerNormalized.begin(), lowerNormalized.end(), lowerNormalized.begin(), ::tolower);
    for (const auto& pair : shortNameMap) {
        std::string lowerKey = pair.first;
        std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
        if (lowerKey == lowerNormalized) {
            return pair.second;
        }
    }
    
    // Try case-insensitive match for full names (values)
    // This handles cases where user types lowercase full name like "genesis" instead of "Genesis"
    for (const auto& pair : shortNameMap) {
        std::string lowerValue = pair.second;
        std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
        if (lowerValue == lowerNormalized) {
            return pair.second; // Return the properly capitalized full name
        }
    }
    
    // No mapping found, return original (case-insensitive comparison in chapterContainsBookName will handle it)
    return shortName;
}

/**
 * Finds the book directory by name
 * @param booksDir The path to the books directory
 * @param bookName The name of the book to find (can be short name or full name)
 * @return The path to the book directory, or empty if not found
 */
static std::filesystem::path findBookDirByName(const std::filesystem::path& booksDir, const std::string& bookName) {
    // First try to expand short name to full name
    std::string expandedName = expandBookName(bookName);
    
    // Iterate through the books directory and check if the book name matches.
    for (const auto& bEntry : listDirectory(booksDir)) {
        if (!bEntry.is_directory()) continue;
        const auto chaptersDir = bEntry.path() / "chapters";
        if (!isDirectory(chaptersDir)) continue;
        // Check any chapter file for matching book name
        for (const auto& cEntry : listDirectory(chaptersDir)) {
            if (!cEntry.is_regular_file()) continue;
            // Try expanded name first (this handles both short names and full names)
            if (chapterContainsBookName(cEntry.path(), expandedName)) return bEntry.path();
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
                    // Format in prose style
                    int terminalWidth = getTerminalWidth();
                    int leftMargin = std::max(4, std::min(15, static_cast<int>(terminalWidth * 0.10)));
                    int firstLineIndent = 4;
                    std::string formatted = formatVerseProse(std::string(v["text"].GetString()), 
                                                             verseNum, terminalWidth, leftMargin, firstLineIndent);
                    printOut(formatted);
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
    if (startVerse > endVerse) return false; // allow start == end for single verse ranges
    return true;
}

// Parses verse specification: single number, range "a-b", or comma-separated list "23, 27, 34-40"
// Returns sorted vector of verse numbers to print
static bool parseVerseSpec(const std::string& verseSpec, std::vector<int>& verses) {
    if (verseSpec.empty()) {
        return true; // Empty means all verses (entire chapter)
    }
    
    std::string spec = verseSpec;
    // Remove spaces around commas
    size_t pos = 0;
    while ((pos = spec.find(", ", pos)) != std::string::npos) {
        spec.replace(pos, 2, ",");
    }
    while ((pos = spec.find(" ,")) != std::string::npos) {
        spec.replace(pos, 2, ",");
    }
    
    // Split by commas
    std::vector<std::string> parts;
    std::string current;
    for (char c : spec) {
        if (c == ',') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else if (!std::isspace(c)) {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    
    std::set<int> verseSet; // Use set to automatically handle duplicates and sorting
    
    for (const auto& part : parts) {
        if (part.empty()) continue;
        
        // Check if it's a range
        auto dashPos = part.find('-');
        if (dashPos != std::string::npos) {
            int startVerse = 0, endVerse = 0;
            std::string rangeStr = part;
            if (parseVerseRange(rangeStr, startVerse, endVerse)) {
                for (int v = startVerse; v <= endVerse; ++v) {
                    verseSet.insert(v);
                }
            } else {
                return false; // Invalid range format
            }
        } else {
            // Single verse number
            if (!std::all_of(part.begin(), part.end(), ::isdigit)) {
                return false; // Invalid verse number
            }
            try {
                int verseNum = std::stoi(part);
                if (verseNum <= 0) return false;
                verseSet.insert(verseNum);
            } catch (...) {
                return false;
            }
        }
    }
    
    verses.assign(verseSet.begin(), verseSet.end());
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

// Reads and prints a list of verses (may be non-contiguous) from a chapter for a specific version.
static int readVerseListInVersion(const std::filesystem::path& versionRoot, const std::string& bookName, int chapter, const std::vector<int>& verses, bool printAllVerses) {
    const auto booksDir = versionRoot / "books";
    if (!isDirectory(booksDir)) return 3;

    const auto bookDir = findBookDirByName(booksDir, bookName);
    if (bookDir.empty()) return 4;

    const auto chapterFile = bookDir / "chapters" / (std::to_string(chapter) + ".json");
    if (!isRegularFile(chapterFile)) {
        printErr("chapter not found: " + std::to_string(chapter) + "\n");
        return 3;
    }
    if (fileSize(chapterFile) == 0) {
        printErr("chapter is empty: " + chapterFile.string() + "\n");
        return 3;
    }

    // Parse chapter file
    std::unordered_map<int, std::string> verseToText;
    const int maxVerse = parseChapterFile(chapterFile, &verseToText);
    if (maxVerse == 0) {
        printErr("invalid chapter JSON schema: " + chapterFile.string() + "\n");
        return 2;
    }

    // Determine which verses to print
    std::vector<int> versesToPrint;
    if (printAllVerses) {
        // Print all verses from 1 to maxVerse
        for (int v = 1; v <= maxVerse; ++v) {
            if (verseToText.find(v) != verseToText.end()) {
                versesToPrint.push_back(v);
            }
        }
    } else {
        // Validate that all requested verses exist and are within range
        for (int v : verses) {
            if (v > maxVerse) {
                printErr("verse " + std::to_string(v) + " exceeds available verses (max " + std::to_string(maxVerse) + ")\n");
                return 3;
            }
            if (verseToText.find(v) == verseToText.end()) {
                printErr("verse not found: " + std::to_string(v) + "\n");
                return 3;
            }
        }
        versesToPrint = verses;
    }

    if (versesToPrint.empty()) {
        printErr("no verses found in chapter " + std::to_string(chapter) + "\n");
        return 3;
    }

    // Calculate formatting parameters once
    int terminalWidth = getTerminalWidth();
    int leftMargin = std::max(4, std::min(15, static_cast<int>(terminalWidth * 0.10)));
    int firstLineIndent = 4;

    // Print verses
    bool isFirstVerse = true;
    for (int verseNum : versesToPrint) {
        auto it = verseToText.find(verseNum);
        if (it == verseToText.end()) continue;

        // Add blank line before verse if not the first one
        if (!isFirstVerse) {
            printOut("\n");
        }
        isFirstVerse = false;

        // Format verse in prose style
        std::string formatted = formatVerseProse(it->second, verseNum, terminalWidth, leftMargin, firstLineIndent);
        printOut(formatted);
    }
    return 0;
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

    // Calculate formatting parameters once
    int terminalWidth = getTerminalWidth();
    int leftMargin = std::max(4, std::min(15, static_cast<int>(terminalWidth * 0.10)));
    int firstLineIndent = 4;

    bool allFound = true;
    bool isFirstVerse = true;
    for (int i = startVerse; i <= endVerse; ++i) {
        auto it = verseToText.find(i);
        if (it == verseToText.end()) {
            allFound = false;
            printErr("verse not found: " + std::to_string(i) + "\n");
            continue;
        }
        
        // Add blank line before verse if not the first one
        if (!isFirstVerse) {
            printOut("\n");
        }
        isFirstVerse = false;
        
        // Format verse in prose style
        std::string formatted = formatVerseProse(it->second, i, terminalWidth, leftMargin, firstLineIndent);
        printOut(formatted);
    }
    return allFound ? 0 : 3;
}

/**
 * Reads verses based on verseSpec. If verseSpec is empty, prints entire chapter.
 * verseSpec can be: single number, range "a-b", or comma-separated list "23, 27, 34-40"
 */
int readVersesFromSpec(const std::filesystem::path& root, const std::string& bookName, int chapter, const std::string& verseSpec) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }

    // Parse verse specification
    std::vector<int> verses;
    bool printAllVerses = verseSpec.empty();
    if (!printAllVerses) {
        if (!parseVerseSpec(verseSpec, verses)) {
            printErr("invalid verse specification format. Expected: single number, range (a-b), or comma-separated list (e.g., \"23, 27, 34-40\")\n");
            return 1;
        }
    }

    // Iterate through versions until one succeeds
    for (const auto& version : listDirectory(root)) {
        if (!version.is_directory()) continue;
        int rc = readVerseListInVersion(version.path(), bookName, chapter, verses, printAllVerses);
        if (rc == 0) return 0;
        if (rc == 2) return rc; // schema error
        // otherwise try next version
    }
    return 4;
}

/**
 * Reads verses based on verseSpec for a specific version
 */
int readVersesFromSpec(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, const std::string& verseSpec) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }

    // Parse verse specification
    std::vector<int> verses;
    bool printAllVerses = verseSpec.empty();
    if (!printAllVerses) {
        if (!parseVerseSpec(verseSpec, verses)) {
            printErr("invalid verse specification format. Expected: single number, range (a-b), or comma-separated list (e.g., \"23, 27, 34-40\")\n");
            return 1;
        }
    }

    if (!versionId.empty()) {
        const auto versionRoot = root / versionId;
        if (!isDirectory(versionRoot)) {
            printErr("version not found: " + versionId + "\n");
            return 3;
        }
        return readVerseListInVersion(versionRoot, bookName, chapter, verses, printAllVerses);
    }
    return readVersesFromSpec(root, bookName, chapter, verseSpec);
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