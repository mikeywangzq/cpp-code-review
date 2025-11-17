#include "parser/ast_parser.h"
#include "rules/rule_engine.h"
#include "report/reporter.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

namespace cpp_review {

// AnalysisConsumer implementation
AnalysisConsumer::AnalysisConsumer(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

void AnalysisConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    // Run all registered rules on this translation unit
    rule_engine_.runAllRules(&context, reporter_);
}

// AnalysisAction implementation
AnalysisAction::AnalysisAction(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

std::unique_ptr<clang::ASTConsumer> AnalysisAction::CreateASTConsumer(
    clang::CompilerInstance& compiler,
    llvm::StringRef file) {
    return std::make_unique<AnalysisConsumer>(rule_engine_, reporter_);
}

// AnalysisActionFactory implementation
AnalysisActionFactory::AnalysisActionFactory(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

std::unique_ptr<clang::FrontendAction> AnalysisActionFactory::create() {
    return std::make_unique<AnalysisAction>(rule_engine_, reporter_);
}

// ASTParser implementation
ASTParser::ASTParser(const std::vector<std::string>& source_paths,
                     const std::string& cpp_standard)
    : source_paths_(source_paths), cpp_standard_(cpp_standard) {}

bool ASTParser::parse(RuleEngine& engine, Reporter& reporter) {
    // Create compilation database
    std::vector<std::string> args;
    args.push_back("cpp-agent");

    for (const auto& path : source_paths_) {
        args.push_back(path);
    }

    args.push_back("--");
    args.push_back("-std=" + cpp_standard_);
    args.push_back("-fsyntax-only");
    args.push_back("-w"); // Suppress warnings from compilation

    // Convert to char* array
    std::vector<const char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(arg.c_str());
    }

    int argc = static_cast<int>(c_args.size());

    // Parse command line options
    llvm::cl::OptionCategory category("cpp-agent options");
    auto expected_parser = clang::tooling::CommonOptionsParser::create(
        argc, c_args.data(), category);

    if (!expected_parser) {
        std::cerr << "Error creating parser: "
                 << llvm::toString(expected_parser.takeError()) << "\n";
        return false;
    }

    clang::tooling::CommonOptionsParser& options_parser = expected_parser.get();
    clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                   options_parser.getSourcePathList());

    // Run the tool with our action factory
    AnalysisActionFactory factory(engine, reporter);
    int result = tool.run(&factory);

    return result == 0;
}

} // namespace cpp_review
