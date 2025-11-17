#pragma once

#include "rules/rule.h"
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>

namespace cpp_review {

class AssignmentInConditionVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitIfStmt(clang::IfStmt* stmt);
    bool VisitWhileStmt(clang::WhileStmt* stmt);
    bool VisitForStmt(clang::ForStmt* stmt);

private:
    void checkCondition(clang::Expr* cond, clang::SourceLocation loc);
    bool hasAssignment(clang::Expr* expr);
};

class AssignmentInConditionRule : public Rule {
public:
    std::string getRuleId() const override { return "ASSIGN-COND-001"; }
    std::string getRuleName() const override { return "Assignment in Condition"; }
    std::string getDescription() const override {
        return "Detects assignment operators (=) used in conditional statements where comparison (==) was likely intended";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
