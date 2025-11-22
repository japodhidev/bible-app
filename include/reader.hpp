#pragma once

#include <filesystem>
#include <string>

// Reads verses based on verseSpec. If verseSpec is empty, prints entire chapter.
// verseSpec can be: single number, range "a-b", or comma-separated list "23, 27, 34-40"
int readVersesFromSpec(const std::filesystem::path& root, const std::string& bookName, int chapter, const std::string& verseSpec);
int readVersesFromSpec(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, const std::string& verseSpec);

// Legacy functions (kept for compatibility, but prefer readVersesFromSpec)
int readVerse(const std::filesystem::path& root, const std::string& bookName, int chapter, int verse);
int readVerse(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, int verse);
int readVerses(const std::filesystem::path& root, const std::string& bookName, int chapter, const std::string& verseRange);
int readVerses(const std::filesystem::path& root, const std::string& versionId, const std::string& bookName, int chapter, const std::string& verseRange);