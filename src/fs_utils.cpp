#include "../include/fs_utils.hpp"

#include <regex>

/** 
 * Checks if the path is a directory
 * @param p The path to check
 * @return True if the path is a directory, false otherwise
 */
bool isDirectory(const std::filesystem::path& p) {
    std::error_code ec;
    return std::filesystem::is_directory(p, ec) && !ec;
}

/**
 * Checks if the path is a regular file
 * @param p The path to check
 * @return True if the path is a regular file, false otherwise
 */
bool isRegularFile(const std::filesystem::path& p) {
    std::error_code ec;
    return std::filesystem::is_regular_file(p, ec) && !ec;
}

/**
 * Lists the contents of a directory
 * @param p The path to the directory
 * @return A vector of directory entries
 */
std::vector<std::filesystem::directory_entry> listDirectory(const std::filesystem::path& p) {
    std::vector<std::filesystem::directory_entry> entries;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(p, ec)) {
        if (ec) break;
        entries.push_back(entry);
    }
    return entries;
}

/**
 * Checks if the filename matches the numeric JSON pattern
 * @param filename The filename to check
 * @return True if the filename matches the numeric JSON pattern, false otherwise
 */
bool matchesNumericJson(const std::string& filename) {
    static const std::regex re("^[1-9][0-9]*\\.json$");
    return std::regex_match(filename, re);
}

/**
 * Gets the size of a file
 * @param p The path to the file
 * @return The size of the file
 */
std::uintmax_t fileSize(const std::filesystem::path& p) {
    std::error_code ec;
    auto sz = std::filesystem::file_size(p, ec);
    if (ec) return 0;
    return sz;
}


