#pragma once

#include <filesystem>
#include <string>
#include <vector>

bool isDirectory(const std::filesystem::path& p);
bool isRegularFile(const std::filesystem::path& p);
std::vector<std::filesystem::directory_entry> listDirectory(const std::filesystem::path& p);
bool matchesNumericJson(const std::string& filename);
std::uintmax_t fileSize(const std::filesystem::path& p);

#pragma once

#include <filesystem>
#include <string>
#include <vector>

bool isDirectory(const std::filesystem::path& p);
bool isRegularFile(const std::filesystem::path& p);
std::vector<std::filesystem::directory_entry> listDirectory(const std::filesystem::path& p);
bool matchesNumericJson(const std::string& filename);


