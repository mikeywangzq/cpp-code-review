#pragma once

#include "rule.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <map>
#include <set>

namespace cpp_review {

class UseAfterFreeVisitor : public clang::RecursiveASTVisitor<UseAfterFreeVisitor> {
public:
    explicit UseAfterFreeVisitor(clang::ASTContext* context, Reporter& reporter)
        : context_(context), reporter_(reporter) {}

    bool VisitFunctionDecl(clang::FunctionDecl* func);

private:
    void analyzeFunctionBody(clang::Stmt* body);
    void analyzeStmt(clang::Stmt* stmt);
    void checkDeleteExpr(clang::CXXDeleteExpr* deleteExpr);
    void checkPointerUsage(clang::Expr* expr);
    bool isPointerDeleted(const clang::ValueDecl* decl);
    const clang::ValueDecl* getReferencedDecl(clang::Expr* expr);

    clang::ASTContext* context_;
    Reporter& reporter_;

    // Track deleted pointers in current function scope
    std::set<const clang::ValueDecl*> deletedPointers_;
    std::map<const clang::ValueDecl*, clang::SourceLocation> deleteLocations_;
};

class UseAfterFreeRule : public Rule {
public:
    std::string getRuleId() const override {
        return "USE-AFTER-FREE-001";
    }

    std::string getRuleName() const override {
        return "Use-After-Free Detection";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override {
        UseAfterFreeVisitor visitor(context, reporter);
        visitor.TraverseDecl(context->getTranslationUnitDecl());
    }
};

} // namespace cpp_review
