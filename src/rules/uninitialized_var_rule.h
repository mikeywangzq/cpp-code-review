#pragma once

#include "rules/rule.h"
#include <clang/AST/Decl.h>

namespace cpp_review {

class UninitializedVarVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitVarDecl(clang::VarDecl* decl);

private:
    bool isBuiltinType(clang::QualType type);
    bool shouldCheckVariable(clang::VarDecl* decl);
};

class UninitializedVarRule : public Rule {
public:
    std::string getRuleId() const override { return "UNINIT-VAR-001"; }
    std::string getRuleName() const override { return "Uninitialized Variable"; }
    std::string getDescription() const override {
        return "Detects potentially uninitialized variables";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
