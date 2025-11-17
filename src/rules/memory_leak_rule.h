#pragma once

#include "rules/rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <map>
#include <set>

namespace cpp_review {

class MemoryLeakVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitCXXNewExpr(clang::CXXNewExpr* newExpr);
    bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* deleteExpr);
    bool VisitReturnStmt(clang::ReturnStmt* returnStmt);
    bool VisitVarDecl(clang::VarDecl* decl);

private:
    struct AllocationInfo {
        clang::SourceLocation location;
        std::string variable_name;
        bool is_deleted = false;
        bool is_returned = false;
    };

    std::map<const clang::VarDecl*, AllocationInfo> allocations_;

    void checkForLeak(const clang::VarDecl* decl, const AllocationInfo& info);
    const clang::VarDecl* getVarDeclFromExpr(clang::Expr* expr);
};

class MemoryLeakRule : public Rule {
public:
    std::string getRuleId() const override { return "MEMORY-LEAK-001"; }
    std::string getRuleName() const override { return "Memory Leak Detection"; }
    std::string getDescription() const override {
        return "Detects potential memory leaks from new/delete mismatches";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
