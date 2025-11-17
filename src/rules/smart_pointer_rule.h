#pragma once

#include "rules/rule.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>

namespace cpp_review {

class SmartPointerVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitVarDecl(clang::VarDecl* decl);
    bool VisitCXXNewExpr(clang::CXXNewExpr* newExpr);

private:
    bool isRawPointerWithNew(clang::VarDecl* decl);
    bool shouldUseUniquePtr(clang::VarDecl* decl);
    bool shouldUseSharedPtr(clang::VarDecl* decl);
};

class SmartPointerRule : public Rule {
public:
    std::string getRuleId() const override { return "SMART-PTR-001"; }
    std::string getRuleName() const override { return "Smart Pointer Suggestion"; }
    std::string getDescription() const override {
        return "Suggests using smart pointers instead of raw pointers for better memory safety";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
