#pragma once

#include "rules/rule.h"
#include <clang/AST/Stmt.h>
#include <clang/AST/Decl.h>

namespace cpp_review {

class LoopCopyVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitForStmt(clang::ForStmt* forStmt);
    bool VisitCXXForRangeStmt(clang::CXXForRangeStmt* rangeStmt);
    bool VisitWhileStmt(clang::WhileStmt* whileStmt);

private:
    void checkLoopBody(clang::Stmt* body, clang::SourceLocation loopLoc);
    bool isExpensiveCopy(clang::VarDecl* decl);
    bool isContainerType(clang::QualType type);
    bool isClassType(clang::QualType type);

    class LoopBodyVisitor : public clang::RecursiveASTVisitor<LoopBodyVisitor> {
    public:
        LoopBodyVisitor(LoopCopyVisitor* parent) : parent_(parent) {}

        bool VisitVarDecl(clang::VarDecl* decl);

    private:
        LoopCopyVisitor* parent_;
    };
};

class LoopCopyRule : public Rule {
public:
    std::string getRuleId() const override { return "LOOP-COPY-001"; }
    std::string getRuleName() const override { return "Expensive Loop Copy"; }
    std::string getDescription() const override {
        return "Detects expensive copy operations inside loops";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
