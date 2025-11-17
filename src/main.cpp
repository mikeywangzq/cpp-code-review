#include "cli/cli.h"
#include "parser/ast_parser.h"
#include "rules/rule_engine.h"
#include "rules/null_pointer_rule.h"
#include "rules/uninitialized_var_rule.h"
#include "rules/assignment_in_condition_rule.h"
#include "rules/unsafe_c_functions_rule.h"
#include "report/reporter.h"

#include <iostream>
#include <memory>

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

    std::cout << "╔══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║      C++ Code Review Agent - Starting Analysis                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Configuration:\n";
    std::cout << "  C++ Standard: " << options.cpp_standard << "\n";
    std::cout << "  Files to analyze: " << options.source_paths.size() << "\n";
    std::cout << "\n";

    // Print files being analyzed
    std::cout << "Files:\n";
    for (const auto& path : options.source_paths) {
        std::cout << "  - " << path << "\n";
    }
    std::cout << "\n";

    // Create rule engine and register all rules
    RuleEngine engine;
    engine.registerRule(std::make_unique<NullPointerRule>());
    engine.registerRule(std::make_unique<UninitializedVarRule>());
    engine.registerRule(std::make_unique<AssignmentInConditionRule>());
    engine.registerRule(std::make_unique<UnsafeCFunctionsRule>());

    std::cout << "Registered " << engine.getRuleCount() << " analysis rules\n";
    std::cout << "\n";
    std::cout << "Analyzing...\n";

    // Create reporter
    Reporter reporter;

    // Create AST parser and run analysis
    ASTParser parser(options.source_paths, options.cpp_standard);
    bool success = parser.parse(engine, reporter);

    if (!success) {
        std::cerr << "\nError: Analysis failed\n";
        return 1;
    }

    // Generate and display report
    reporter.generateReport(std::cout);

    // Return error code if critical issues found
    if (reporter.getCriticalCount() > 0) {
        return 2;
    }

    return 0;
}
