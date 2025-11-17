#include "cli/cli.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace cpp_review {

namespace fs = std::filesystem;

bool CLI::isSourceFile(const std::string& path) {
    static const std::vector<std::string> extensions = {
        ".cpp", ".cc", ".cxx", ".c++",
        ".h", ".hpp", ".hxx", ".h++"
    };

    fs::path p(path);
    std::string ext = p.extension().string();

    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

CLIOptions CLI::parseArguments(int argc, char* argv[]) {
    CLIOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            options.help = true;
            return options;
        }
        else if (arg == "-v" || arg == "--version") {
            options.version = true;
            return options;
        }
        else if (arg == "--std" && i + 1 < argc) {
            options.cpp_standard = argv[++i];
        }
        else if (arg.find("--std=") == 0) {
            options.cpp_standard = arg.substr(6);
        }
        else if (arg == "--html" || arg == "--generate-html") {
            options.generate_html = true;
        }
        else if (arg == "--html-output" && i + 1 < argc) {
            options.html_output = argv[++i];
            options.generate_html = true;
        }
        else if (arg.find("--html-output=") == 0) {
            options.html_output = arg.substr(14);
            options.generate_html = true;
        }
        else if (arg == "scan" && i + 1 < argc) {
            // Next argument should be the path
            ++i;
            std::string path = argv[i];

            if (fs::is_directory(path)) {
                // Recursively find all C++ source files
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file() && isSourceFile(entry.path().string())) {
                        options.source_paths.push_back(entry.path().string());
                    }
                }
            }
            else if (fs::is_regular_file(path) && isSourceFile(path)) {
                options.source_paths.push_back(path);
            }
            else {
                std::cerr << "Warning: " << path << " is not a valid C++ source file or directory\n";
            }
        }
        else if (arg == "--") {
            // Everything after -- is for the compiler
            break;
        }
        else if (isSourceFile(arg)) {
            // Direct file path
            options.source_paths.push_back(arg);
        }
    }

    return options;
}

void CLI::printHelp() {
    std::cout << R"(
C++ Code Review Agent - Static Analysis Tool
Version 2.0.0

USAGE:
    cpp-agent scan <path> [options]
    cpp-agent <file.cpp> [options]

COMMANDS:
    scan <path>         Scan a C++ file or directory

OPTIONS:
    --std=<standard>        Specify C++ standard (default: c++17)
                            Examples: c++11, c++14, c++17, c++20
    --html                  Generate HTML report
    --html-output=<file>    HTML report output file (default: report.html)
    -h, --help              Display this help message
    -v, --version           Display version information

EXAMPLES:
    # Scan a single file
    cpp-agent scan example.cpp

    # Scan an entire directory
    cpp-agent scan /path/to/project

    # Generate HTML report
    cpp-agent scan main.cpp --html

    # Specify custom HTML output
    cpp-agent scan main.cpp --html-output=my_report.html

    # Specify C++ standard
    cpp-agent scan main.cpp --std=c++20

    # Direct file analysis
    cpp-agent main.cpp --std=c++17

DETECTED ISSUES (V2.0):
    Bug Detection (V1.0):
    - Null pointer dereferences
    - Uninitialized variables
    - Assignment in conditional expressions (if (a = b))
    - Unsafe C-style functions (strcpy, sprintf, etc.)

    Performance Analysis (V1.5):
    - Memory leaks (new/delete mismatch)
    - Smart pointer suggestions
    - Expensive copy operations in loops

    Advanced Security (V2.0 NEW):
    - Integer overflow detection
    - Use-after-free detection
    - Buffer overflow detection

CONFIGURATION:
    Place a .cpp-agent.yml file in your project root to configure:
    - disabled_rules: [RULE-ID-001, RULE-ID-002]
    - html_output: true
    - cpp_standard: c++20

For more information, visit: https://github.com/yourusername/cpp-code-review
)";
}

void CLI::printVersion() {
    std::cout << "C++ Code Review Agent v2.0.0\n";
    std::cout << "Built with Clang/LLVM AST analysis\n";
    std::cout << "New in v2.0: Integer overflow, Use-after-free, Buffer overflow detection\n";
    std::cout << "v1.5 features: Memory leak detection, Smart pointers, Loop optimization\n";
}

} // namespace cpp_review
