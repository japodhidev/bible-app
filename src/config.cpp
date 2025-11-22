#include "../include/config.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#endif

// Simple, fast INI parser (no external dependencies)
static void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

Config loadConfig(const std::filesystem::path& configPath) {
    Config config{};
    
    if (!std::filesystem::exists(configPath) || !std::filesystem::is_regular_file(configPath)) {
        return config; // File doesn't exist, return empty config
    }
    
    std::ifstream file(configPath);
    if (!file.is_open()) {
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        trim(line);
        if (line.empty()) continue;
        
        // Parse key=value
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        trim(key);
        trim(value);
        
        // Remove quotes if present
        if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"') {
            value = value.substr(1, value.size() - 2);
        }
        
        std::string keyLower = toLower(key);
        
        if (keyLower == "resources_dir" || keyLower == "resources_directory" || keyLower == "resources") {
            config.resourcesRoot = value;
        } else if (keyLower == "default_version" || keyLower == "version" || keyLower == "default_bible_version") {
            config.defaultVersion = value;
        }
    }
    
    return config;
}

std::filesystem::path getDefaultConfigPath() {
#ifdef _WIN32
    // Windows: %APPDATA%/bible-app/config.ini
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataPath))) {
        std::filesystem::path configDir = std::filesystem::path(appDataPath) / "bible-app";
        return configDir / "config.ini";
    }
    // Fallback to current directory
    return std::filesystem::path("config.ini");
#else
    // Unix-like: ~/.config/bible-app/config.ini or ~/.bible-app.ini
    const char* home = std::getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    
    if (home) {
        // Try XDG config first
        const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
        if (xdgConfig) {
            return std::filesystem::path(xdgConfig) / "bible-app" / "config.ini";
        }
        // Fallback to ~/.config/bible-app/config.ini
        return std::filesystem::path(home) / ".config" / "bible-app" / "config.ini";
    }
    
    // Last resort: current directory
    return std::filesystem::path("config.ini");
#endif
}


