// C++ 代码审查 Agent 主程序
// 基于 Clang/LLVM AST 的静态分析工具

// 命令行接口
#include "cli/cli.h"
// AST 解析器
#include "parser/ast_parser.h"
// 规则引擎
#include "rules/rule_engine.h"

// V1.0 基础检测规则
#include "rules/null_pointer_rule.h"
#include "rules/uninitialized_var_rule.h"
#include "rules/assignment_in_condition_rule.h"
#include "rules/unsafe_c_functions_rule.h"

// V1.5 性能分析规则
#include "rules/memory_leak_rule.h"
#include "rules/smart_pointer_rule.h"
#include "rules/loop_copy_rule.h"

// V2.0 高级安全分析规则
#include "rules/integer_overflow_rule.h"
#include "rules/use_after_free_rule.h"
#include "rules/buffer_overflow_rule.h"

// 报告生成器
#include "report/reporter.h"
#include "report/html_reporter.h"
// 配置管理
#include "config/config.h"
// Git 集成 (V1.5)
#include "git/git_integration.h"

#include <iostream>
#include <memory>
#include <filesystem>
#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
    using namespace cpp_review;

    // 解析命令行参数
    CLIOptions options = CLI::parseArguments(argc, argv);

    // 如果请求帮助信息,显示后退出
    if (options.help) {
        CLI::printHelp();
        return 0;
    }

    // 如果请求版本信息,显示后退出
    if (options.version) {
        CLI::printVersion();
        return 0;
    }

    // ===== V1.5 Git 集成: 增量分析 =====
    std::optional<PREnvironment> pr_env;

    if (options.incremental) {
        // 检查是否为 Git 仓库
        if (!GitIntegration::isGitRepository()) {
            std::cerr << "Error: Not a Git repository. Incremental analysis requires Git.\n";
            return 1;
        }

        // 转换增量模式
        IncrementalMode mode = IncrementalMode::WORKSPACE;
        if (options.incremental_mode == "workspace") {
            mode = IncrementalMode::WORKSPACE;
        } else if (options.incremental_mode == "staged") {
            mode = IncrementalMode::STAGED;
        } else if (options.incremental_mode == "branch") {
            mode = IncrementalMode::BRANCH;
        } else if (options.incremental_mode == "commit") {
            mode = IncrementalMode::COMMIT;
        } else if (options.incremental_mode == "pr") {
            mode = IncrementalMode::PR;
            pr_env = GitIntegration::detectPREnvironment();
        }

        // 获取变更文件列表
        std::cout << "🔍 Git incremental analysis mode: " << options.incremental_mode << "\n";
        if (!options.git_reference.empty()) {
            std::cout << "   Reference: " << options.git_reference << "\n";
        }

        auto changed_files = GitIntegration::getChangedFiles(mode, options.git_reference);

        if (changed_files.empty()) {
            std::cout << "✅ No C++ files changed. Nothing to analyze.\n";
            return 0;
        }

        std::cout << "   Found " << changed_files.size() << " changed C++ file(s)\n\n";
        options.source_paths = changed_files;
    }

    // 检查是否指定了源文件
    if (options.source_paths.empty()) {
        std::cerr << "Error: No source files specified\n";
        std::cerr << "Use 'cpp-agent --help' for usage information\n";
        return 1;
    }

    // 加载配置文件
    Config config;
    if (std::filesystem::exists(".cpp-agent.yml")) {
        std::cout << "Loading configuration from .cpp-agent.yml...\n";
        config = ConfigManager::loadConfig(".cpp-agent.yml");
    } else {
        // 使用默认配置
        config = ConfigManager::getDefaultConfig();
    }

    // 命令行选项覆盖配置文件设置
    if (!options.cpp_standard.empty()) {
        config.cpp_standard = options.cpp_standard;
    }
    if (options.generate_html) {
        config.generate_html = true;
        if (!options.html_output.empty()) {
            config.html_output_file = options.html_output;
        }
    }

    // 显示启动信息
    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║      C++ Code Review Agent V2.0 - Starting Analysis             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Configuration:\n";
    std::cout << "  C++ Standard: " << config.cpp_standard << "\n";
    std::cout << "  Files to analyze: " << options.source_paths.size() << "\n";
    std::cout << "  HTML Report: " << (config.generate_html ? "Yes (" + config.html_output_file + ")" : "No") << "\n";
    std::cout << "\n";

    // 显示待分析的文件列表
    std::cout << "Files:\n";
    for (const auto& path : options.source_paths) {
        std::cout << "  - " << path << "\n";
    }
    std::cout << "\n";

    // 创建规则引擎并注册所有检测规则
    RuleEngine engine;

    // ===== V1.0 基础检测规则 =====
    // 空指针解引用检测
    if (config.disabled_rules.find("NULL-PTR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<NullPointerRule>());
    }
    // 未初始化变量检测
    if (config.disabled_rules.find("UNINIT-VAR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UninitializedVarRule>());
    }
    // 赋值/比较混淆检测
    if (config.disabled_rules.find("ASSIGN-COND-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<AssignmentInConditionRule>());
    }
    // 不安全 C 函数检测
    if (config.disabled_rules.find("UNSAFE-C-FUNC-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UnsafeCFunctionsRule>());
    }

    // ===== V1.5 性能分析规则 =====
    // 内存泄漏检测
    if (config.disabled_rules.find("MEMORY-LEAK-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<MemoryLeakRule>());
    }
    // 智能指针建议
    if (config.disabled_rules.find("SMART-PTR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<SmartPointerRule>());
    }
    // 循环拷贝优化
    if (config.disabled_rules.find("LOOP-COPY-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<LoopCopyRule>());
    }

    // ===== V2.0 高级安全分析规则 =====
    // 整数溢出检测
    if (config.disabled_rules.find("INTEGER-OVERFLOW-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<IntegerOverflowRule>());
    }
    // Use-After-Free 检测
    if (config.disabled_rules.find("USE-AFTER-FREE-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UseAfterFreeRule>());
    }
    // 缓冲区溢出检测
    if (config.disabled_rules.find("BUFFER-OVERFLOW-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<BufferOverflowRule>());
    }

    std::cout << "Registered " << engine.getRuleCount() << " analysis rules (V2.0)\n";
    std::cout << "\n";
    std::cout << "Analyzing...\n";

    // 创建报告生成器
    Reporter reporter;

    // 创建 AST 解析器并运行分析
    ASTParser parser(options.source_paths, config.cpp_standard);
    bool success = parser.parse(engine, reporter);

    if (!success) {
        std::cerr << "\nError: Analysis failed\n";
        return 1;
    }

    // 生成并显示控制台报告
    reporter.generateReport(std::cout);

    // 如果请求,生成 HTML 报告
    if (config.generate_html) {
        std::cout << "\nGenerating HTML report: " << config.html_output_file << "\n";
        try {
            HTMLReporter::generateHTMLReport(reporter, config.html_output_file);
            std::cout << "✓ HTML report generated successfully!\n";
        } catch (const std::exception& e) {
            std::cerr << "Error generating HTML report: " << e.what() << "\n";
        }
    }

    // ===== V1.5 Git 集成: PR 评论生成 =====
    if (options.pr_mode || !options.pr_comment_file.empty()) {
        std::cout << "\n📝 Generating PR review comment...\n";

        // 生成报告内容
        std::ostringstream report_stream;
        reporter.generateReport(report_stream);
        std::string report_content = report_stream.str();

        // 如果在 PR 环境中,使用 PR 环境信息
        std::string pr_comment;
        if (pr_env) {
            pr_comment = GitIntegration::generatePRComment(report_content, *pr_env);
            std::cout << "   PR Environment: " << pr_env->provider << "\n";
            std::cout << "   PR #" << pr_env->pr_number << ": "
                      << pr_env->base_branch << " <- " << pr_env->head_branch << "\n";
        } else {
            // 不在 PR 环境,生成简单格式
            pr_comment = "## 🤖 C++ Code Review Report\n\n" + report_content;
        }

        // 输出到文件或标准输出
        if (!options.pr_comment_file.empty()) {
            std::ofstream comment_file(options.pr_comment_file);
            if (comment_file.is_open()) {
                comment_file << pr_comment;
                std::cout << "✓ PR comment saved to: " << options.pr_comment_file << "\n";
            } else {
                std::cerr << "Error: Cannot write to " << options.pr_comment_file << "\n";
            }
        } else {
            std::cout << "\n" << pr_comment << "\n";
        }
    }

    // 如果发现严重问题,返回错误码
    if (reporter.getCriticalCount() > 0) {
        return 2;
    }

    return 0;
}
