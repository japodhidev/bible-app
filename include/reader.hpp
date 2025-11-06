#pragma once

#include <filesystem>
#include <string>

// Reads and prints the verse text for the given book/chapter/verse. Returns exit code.
int readVerse(const std::filesystem::path& root, const std::string& bookName, int chapter, int verse);
int readVerse(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, int verse);

// Reads and prints multiple verses given as a range string "start-end". Returns exit code.
int readVerses(const std::filesystem::path& root, const std::string& bookName, int chapter, const std::string& verseRange);
int readVerses(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, const std::string& verseRange);