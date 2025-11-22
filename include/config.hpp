#pragma once

#include <filesystem>
#include <string>

struct Config {
    std::filesystem::path resourcesRoot {};
    std::string defaultVersion {};
};

// Loads config from INI file. Returns empty config if file doesn't exist or on error.
Config loadConfig(const std::filesystem::path& configPath);

// Gets the default config file path (e.g., ~/.config/bible-app/config.ini or .bible-app.ini in home)
std::filesystem::path getDefaultConfigPath();


