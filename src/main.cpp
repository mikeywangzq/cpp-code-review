#include "cli/cli.h"
#include "parser/ast_parser.h"
#include "rules/rule_engine.h"
#include "rules/null_pointer_rule.h"
#include "rules/uninitialized_var_rule.h"
#include "rules/assignment_in_condition_rule.h"
#include "rules/unsafe_c_functions_rule.h"
#include "rules/memory_leak_rule.h"
#include "rules/smart_pointer_rule.h"
#include "rules/loop_copy_rule.h"
#include "rules/integer_overflow_rule.h"
#include "rules/use_after_free_rule.h"
#include "rules/buffer_overflow_rule.h"
#include "report/reporter.h"
#include "report/html_reporter.h"
#include "config/config.h"

#include <iostream>
#include <memory>
#include <filesystem>

int main(int argc, char* argv[]) {
    using namespace cpp_review;

    // Parse command line arguments
    CLIOptions options = CLI::parseArguments(argc, argv);

    if (options.help) {
        CLI::printHelp();
        return 0;
    }

    if (options.version) {
        CLI::printVersion();
        return 0;
    }

    if (options.source_paths.empty()) {
        std::cerr << "Error: No source files specified\n";
        std::cerr << "Use 'cpp-agent --help' for usage information\n";
        return 1;
    }

    // Load configuration
    Config config;
    if (std::filesystem::exists(".cpp-agent.yml")) {
        std::cout << "Loading configuration from .cpp-agent.yml...\n";
        config = ConfigManager::loadConfig(".cpp-agent.yml");
    } else {
        config = ConfigManager::getDefaultConfig();
    }

    // Override with command line options
    if (!options.cpp_standard.empty()) {
        config.cpp_standard = options.cpp_standard;
    }
    if (options.generate_html) {
        config.generate_html = true;
        if (!options.html_output.empty()) {
            config.html_output_file = options.html_output;
        }
    }

    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║      C++ Code Review Agent V2.0 - Starting Analysis             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Configuration:\n";
    std::cout << "  C++ Standard: " << config.cpp_standard << "\n";
    std::cout << "  Files to analyze: " << options.source_paths.size() << "\n";
    std::cout << "  HTML Report: " << (config.generate_html ? "Yes (" + config.html_output_file + ")" : "No") << "\n";
    std::cout << "\n";

    // Print files being analyzed
    std::cout << "Files:\n";
    for (const auto& path : options.source_paths) {
        std::cout << "  - " << path << "\n";
    }
    std::cout << "\n";

    // Create rule engine and register all rules
    RuleEngine engine;

    // V1.0 Rules
    if (config.disabled_rules.find("NULL-PTR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<NullPointerRule>());
    }
    if (config.disabled_rules.find("UNINIT-VAR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UninitializedVarRule>());
    }
    if (config.disabled_rules.find("ASSIGN-COND-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<AssignmentInConditionRule>());
    }
    if (config.disabled_rules.find("UNSAFE-C-FUNC-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UnsafeCFunctionsRule>());
    }

    // V1.5 Rules (Performance)
    if (config.disabled_rules.find("MEMORY-LEAK-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<MemoryLeakRule>());
    }
    if (config.disabled_rules.find("SMART-PTR-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<SmartPointerRule>());
    }
    if (config.disabled_rules.find("LOOP-COPY-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<LoopCopyRule>());
    }

    // V2.0 Rules (Advanced Security)
    if (config.disabled_rules.find("INTEGER-OVERFLOW-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<IntegerOverflowRule>());
    }
    if (config.disabled_rules.find("USE-AFTER-FREE-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<UseAfterFreeRule>());
    }
    if (config.disabled_rules.find("BUFFER-OVERFLOW-001") == config.disabled_rules.end()) {
        engine.registerRule(std::make_unique<BufferOverflowRule>());
    }

    std::cout << "Registered " << engine.getRuleCount() << " analysis rules (V2.0)\n";
    std::cout << "\n";
    std::cout << "Analyzing...\n";

    // Create reporter
    Reporter reporter;

    // Create AST parser and run analysis
    ASTParser parser(options.source_paths, config.cpp_standard);
    bool success = parser.parse(engine, reporter);

    if (!success) {
        std::cerr << "\nError: Analysis failed\n";
        return 1;
    }

    // Generate and display report
    reporter.generateReport(std::cout);

    // Generate HTML report if requested
    if (config.generate_html) {
        std::cout << "\nGenerating HTML report: " << config.html_output_file << "\n";
        try {
            HTMLReporter::generateHTMLReport(reporter, config.html_output_file);
            std::cout << "✓ HTML report generated successfully!\n";
        } catch (const std::exception& e) {
            std::cerr << "Error generating HTML report: " << e.what() << "\n";
        }
    }

    // Return error code if critical issues found
    if (reporter.getCriticalCount() > 0) {
        return 2;
    }

    return 0;
}
