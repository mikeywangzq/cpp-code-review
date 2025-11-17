#pragma once

#include <string>
#include <vector>

namespace cpp_review {

struct CLIOptions {
    std::vector<std::string> source_paths;
    std::string cpp_standard = "c++17";
    bool help = false;
    bool version = false;
    bool generate_html = false;
    std::string html_output = "report.html";
    bool enable_ai = false;
    std::string llm_provider = "rule-based";
};

class CLI {
public:
    static CLIOptions parseArguments(int argc, char* argv[]);
    static void printHelp();
    static void printVersion();

private:
    static bool isSourceFile(const std::string& path);
};

} // namespace cpp_review
