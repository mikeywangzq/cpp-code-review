/*
 * 命令行接口实现
 * 处理命令行参数解析和帮助信息显示
 */

#include "cli/cli.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace cpp_review {

namespace fs = std::filesystem;

/**
 * 判断文件是否为 C++ 源文件
 * @param path 文件路径
 * @return 如果是 C++ 源文件返回 true
 */
bool CLI::isSourceFile(const std::string& path) {
    static const std::vector<std::string> extensions = {
        ".cpp", ".cc", ".cxx", ".c++",
        ".h", ".hpp", ".hxx", ".h++"
    };

    fs::path p(path);
    std::string ext = p.extension().string();

    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

/**
 * 解析命令行参数
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 解析后的选项结构
 */
CLIOptions CLI::parseArguments(int argc, char* argv[]) {
    CLIOptions options;

    // 遍历所有命令行参数
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
        // ===== V1.5 Git 集成选项 =====
        else if (arg == "--incremental" || arg == "-i") {
            options.incremental = true;
            options.incremental_mode = "workspace";
        }
        else if (arg.find("--incremental=") == 0) {
            options.incremental = true;
            options.incremental_mode = arg.substr(14);
        }
        else if (arg == "--branch" && i + 1 < argc) {
            options.incremental = true;
            options.incremental_mode = "branch";
            options.git_reference = argv[++i];
        }
        else if (arg.find("--branch=") == 0) {
            options.incremental = true;
            options.incremental_mode = "branch";
            options.git_reference = arg.substr(9);
        }
        else if (arg == "--commit" && i + 1 < argc) {
            options.incremental = true;
            options.incremental_mode = "commit";
            options.git_reference = argv[++i];
        }
        else if (arg.find("--commit=") == 0) {
            options.incremental = true;
            options.incremental_mode = "commit";
            options.git_reference = arg.substr(9);
        }
        else if (arg == "--pr") {
            options.pr_mode = true;
            options.incremental = true;
            options.incremental_mode = "pr";
        }
        else if (arg == "--pr-comment" && i + 1 < argc) {
            options.pr_comment_file = argv[++i];
        }
        else if (arg.find("--pr-comment=") == 0) {
            options.pr_comment_file = arg.substr(13);
        }
        else if (arg == "scan" && i + 1 < argc) {
            // 扫描命令:下一个参数应该是路径
            ++i;
            std::string path = argv[i];

            if (fs::is_directory(path)) {
                // 递归查找目录中的所有 C++ 源文件
                for (const auto& entry : fs::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file() && isSourceFile(entry.path().string())) {
                        options.source_paths.push_back(entry.path().string());
                    }
                }
            }
            else if (fs::is_regular_file(path) && isSourceFile(path)) {
                // 单个文件
                options.source_paths.push_back(path);
            }
            else {
                std::cerr << "Warning: " << path << " is not a valid C++ source file or directory\n";
            }
        }
        else if (arg == "--") {
            // -- 之后的参数传递给编译器
            break;
        }
        else if (isSourceFile(arg)) {
            // 直接指定文件路径
            options.source_paths.push_back(arg);
        }
    }

    return options;
}

/**
 * 打印帮助信息
 * 显示所有可用的命令和选项
 */
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

GIT INTEGRATION OPTIONS (V1.5):
    -i, --incremental       Incremental analysis (workspace changes only)
    --incremental=<mode>    Incremental mode: workspace|staged|branch|commit|pr
    --branch=<name>         Analyze changes vs. specified branch
    --commit=<hash>         Analyze changes since commit
    --pr                    PR review mode (auto-detect base branch)
    --pr-comment=<file>     Output PR comment to file

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

    # Incremental analysis (V1.5)
    cpp-agent --incremental              # Analyze workspace changes
    cpp-agent --branch=main              # Analyze vs. main branch
    cpp-agent --commit=abc123            # Analyze since commit abc123
    cpp-agent --pr                       # PR review mode
    cpp-agent --pr --pr-comment=review.md  # Generate PR comment

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

/**
 * 打印版本信息
 * 显示当前版本号和功能特性
 */
void CLI::printVersion() {
    std::cout << "C++ Code Review Agent v2.0.0\n";
    std::cout << "Built with Clang/LLVM AST analysis\n";
    std::cout << "New in v2.0: Integer overflow, Use-after-free, Buffer overflow detection\n";
    std::cout << "v1.5 features: Memory leak detection, Smart pointers, Loop optimization\n";
}

} // namespace cpp_review
