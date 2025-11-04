#pragma once

#include <filesystem>

// Lists available versions, and if metadata exists, prints version and description
int listResources(const std::filesystem::path& root);
