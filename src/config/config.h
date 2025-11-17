#pragma once

#include <string>
#include <set>
#include <map>

namespace cpp_review {

struct Config {
    // Disabled rules
    std::set<std::string> disabled_rules;

    // Rule severity overrides
    std::map<std::string, std::string> rule_severity;

    // Output options
    bool generate_html = false;
    std::string html_output_file = "report.html";

    // Analysis options
    std::string cpp_standard = "c++17";
    bool verbose = false;

    // LLM Enhancement options
    bool enable_ai_suggestions = false;
    std::string llm_provider = "rule-based";  // "rule-based", "openai", "none"
    std::string llm_api_key = "";             // For OpenAI or other providers
};

class ConfigManager {
public:
    static Config loadConfig(const std::string& config_file);
    static Config getDefaultConfig();

private:
    static std::string trim(const std::string& str);
    static void parseLine(const std::string& line, Config& config);
};

} // namespace cpp_review
