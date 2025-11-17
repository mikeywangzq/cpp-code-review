#pragma once

#include "report/reporter.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <memory>

namespace cpp_review {

// Base class for all analysis rules
class Rule {
public:
    virtual ~Rule() = default;

    virtual std::string getRuleId() const = 0;
    virtual std::string getRuleName() const = 0;
    virtual std::string getDescription() const = 0;

    // Check a single AST node
    virtual void check(clang::ASTContext* context, Reporter& reporter) = 0;
};

// Base visitor class that rules can inherit from
class RuleVisitor : public clang::RecursiveASTVisitor<RuleVisitor> {
public:
    RuleVisitor(clang::ASTContext* context, Reporter& reporter)
        : context_(context), reporter_(reporter) {}

    virtual ~RuleVisitor() = default;

protected:
    clang::ASTContext* context_;
    Reporter& reporter_;

    // Helper method to get source location info
    std::string getFileName(clang::SourceLocation loc) const;
    unsigned getLine(clang::SourceLocation loc) const;
    unsigned getColumn(clang::SourceLocation loc) const;
    std::string getSourceText(clang::SourceRange range) const;
};

} // namespace cpp_review
