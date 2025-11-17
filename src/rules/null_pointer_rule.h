#pragma once

#include "rules/rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <set>

namespace cpp_review {

class NullPointerVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitBinaryOperator(clang::BinaryOperator* op);
    bool VisitUnaryOperator(clang::UnaryOperator* op);
    bool VisitMemberExpr(clang::MemberExpr* expr);
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr);

private:
    void checkForNullDereference(clang::Expr* expr, clang::SourceLocation loc);
    bool isNullPointer(clang::Expr* expr);
    bool isPointerType(clang::Expr* expr);
};

class NullPointerRule : public Rule {
public:
    std::string getRuleId() const override { return "NULL-PTR-001"; }
    std::string getRuleName() const override { return "Null Pointer Dereference"; }
    std::string getDescription() const override {
        return "Detects potential null pointer dereferences";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
