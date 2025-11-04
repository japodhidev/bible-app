#pragma once

#include <filesystem>
#include <string>
#include <vector>

enum class Severity { Warning, Error };

struct ValidationIssue {
    Severity severity {Severity::Error};
    std::filesystem::path path;
    std::string message;
};

struct ValidationResult {
    bool ok {true};
    std::vector<ValidationIssue> issues;
};

ValidationResult validateResources(const std::filesystem::path& root);
int printValidationReport(const ValidationResult& result);