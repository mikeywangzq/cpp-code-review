#pragma once

#include <string>
#include <vector>
#include <memory>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>

namespace cpp_review {

class Reporter;
class RuleEngine;

// AST Consumer that runs our analysis rules
class AnalysisConsumer : public clang::ASTConsumer {
public:
    AnalysisConsumer(RuleEngine& engine, Reporter& reporter);

    void HandleTranslationUnit(clang::ASTContext& context) override;

private:
    RuleEngine& rule_engine_;
    Reporter& reporter_;
};

// Frontend Action for our analysis
class AnalysisAction : public clang::ASTFrontendAction {
public:
    AnalysisAction(RuleEngine& engine, Reporter& reporter);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler,
        llvm::StringRef file) override;

private:
    RuleEngine& rule_engine_;
    Reporter& reporter_;
};

// Factory for creating our actions
class AnalysisActionFactory : public clang::tooling::FrontendActionFactory {
public:
    AnalysisActionFactory(RuleEngine& engine, Reporter& reporter);

    std::unique_ptr<clang::FrontendAction> create() override;

private:
    RuleEngine& rule_engine_;
    Reporter& reporter_;
};

// Main AST Parser class
class ASTParser {
public:
    ASTParser(const std::vector<std::string>& source_paths,
              const std::string& cpp_standard = "c++17");

    bool parse(RuleEngine& engine, Reporter& reporter);

private:
    std::vector<std::string> source_paths_;
    std::string cpp_standard_;
};

} // namespace cpp_review
