#pragma once

#include "rule.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <map>

namespace cpp_review {

class BufferOverflowVisitor : public clang::RecursiveASTVisitor<BufferOverflowVisitor> {
public:
    explicit BufferOverflowVisitor(clang::ASTContext* context, Reporter& reporter)
        : context_(context), reporter_(reporter) {}

    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr);
    bool VisitVarDecl(clang::VarDecl* decl);

private:
    uint64_t getArraySize(const clang::VarDecl* decl);
    bool tryGetConstantIndex(clang::Expr* expr, int64_t& value);

    clang::ASTContext* context_;
    Reporter& reporter_;

    // Track array sizes for declared arrays
    std::map<const clang::VarDecl*, uint64_t> arraySizes_;
};

class BufferOverflowRule : public Rule {
public:
    std::string getRuleId() const override {
        return "BUFFER-OVERFLOW-001";
    }

    std::string getRuleName() const override {
        return "Buffer Overflow Detection";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override {
        BufferOverflowVisitor visitor(context, reporter);
        visitor.TraverseDecl(context->getTranslationUnitDecl());
    }
};

} // namespace cpp_review
