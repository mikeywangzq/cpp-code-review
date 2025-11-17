#include "config/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace cpp_review {

std::string ConfigManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void ConfigManager::parseLine(const std::string& line, Config& config) {
    std::string trimmed = trim(line);

    // Skip empty lines and comments
    if (trimmed.empty() || trimmed[0] == '#') {
        return;
    }

    // Find colon separator
    size_t colon_pos = trimmed.find(':');
    if (colon_pos == std::string::npos) {
        return;
    }

    std::string key = trim(trimmed.substr(0, colon_pos));
    std::string value = trim(trimmed.substr(colon_pos + 1));

    // Parse different configuration options
    if (key == "disabled_rules") {
        // Parse list of rules (comma or space separated)
        std::istringstream iss(value);
        std::string rule;

        // Remove brackets if present
        if (!value.empty() && value[0] == '[') {
            value = value.substr(1);
        }
        if (!value.empty() && value.back() == ']') {
            value.pop_back();
        }

        // Parse comma-separated values
        std::istringstream stream(value);
        while (std::getline(stream, rule, ',')) {
            rule = trim(rule);
            if (!rule.empty()) {
                config.disabled_rules.insert(rule);
            }
        }
    }
    else if (key == "html_output") {
        std::string lower_value = value;
        std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
        config.generate_html = (lower_value == "true" || lower_value == "yes" || lower_value == "1");
    }
    else if (key == "html_output_file") {
        config.html_output_file = value;
    }
    else if (key == "cpp_standard") {
        config.cpp_standard = value;
    }
    else if (key == "verbose") {
        std::string lower_value = value;
        std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
        config.verbose = (lower_value == "true" || lower_value == "yes" || lower_value == "1");
    }
    else if (key.find("severity_") == 0) {
        // Rule severity override: severity_RULE-ID: HIGH
        std::string rule_id = key.substr(9); // Remove "severity_" prefix
        config.rule_severity[rule_id] = value;
    }
}

Config ConfigManager::loadConfig(const std::string& config_file) {
    Config config = getDefaultConfig();

    std::ifstream file(config_file);
    if (!file.is_open()) {
        // Config file not found, use defaults
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        parseLine(line, config);
    }

    return config;
}

Config ConfigManager::getDefaultConfig() {
    Config config;
    config.cpp_standard = "c++17";
    config.generate_html = false;
    config.html_output_file = "report.html";
    config.verbose = false;
    return config;
}

} // namespace cpp_review
