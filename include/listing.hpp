#pragma once

#include <filesystem>

// Lists available versions, prints version and description, and marks default/selected.
int listResources(const std::filesystem::path& root);
int listResources(const std::filesystem::path& root, const std::string& defaultVersionId, const std::string& selectedVersionId);
