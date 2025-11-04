#pragma once

#include <filesystem>
#include <string>

// Reads and prints the verse text for the given book/chapter/verse. Returns exit code.
int readVerse(const std::filesystem::path& root, const std::string& bookName, int chapter, int verse);


