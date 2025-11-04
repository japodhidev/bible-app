#include "../include/listing.hpp"
#include "../include/fs_utils.hpp"
#include "../include/print.hpp"

#include <string>
#include <fstream>

#include <rapidjson/document.h>

/**
 * Prints a line for a version
 * @param translationDir The path to the translation directory
 */
static void printVersionLine(const std::filesystem::path& translationDir) {
    const std::string id = translationDir.filename().string();
    const auto meta = translationDir / (id + ".json");
    std::string version;
    std::string description;

    if (isRegularFile(meta) && fileSize(meta) > 0) {
        std::ifstream in(meta);
        std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        rapidjson::Document d;
        if (!json.empty() && !d.Parse(json.c_str()).HasParseError() && d.IsObject()) {
            if (d.HasMember("version") && d["version"].IsString()) version = d["version"].GetString();
            if (d.HasMember("description") && d["description"].IsString()) description = d["description"].GetString();
        }
    }

    if (!version.empty() || !description.empty()) {
        printOut(id + " | " + version + " | " + description + "\n");
    } else {
        printOut(id + "\n");
    }
}

/**
 * Lists the resources
 * @param root The path to the resources root directory
 * @return The exit code
 */
int listResources(const std::filesystem::path& root) {
    if (!isDirectory(root)) {
        printErr("resources path not found or not a directory: " + root.string() + "\n");
        return 3;
    }

    auto versions = listDirectory(root);
    for (const auto& entry : versions) {
        if (!entry.is_directory()) continue;
        printVersionLine(entry.path());
    }
    return 0;
}


