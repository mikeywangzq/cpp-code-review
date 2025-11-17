#pragma once

#include "rule.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace cpp_review {

class IntegerOverflowVisitor : public clang::RecursiveASTVisitor<IntegerOverflowVisitor> {
public:
    explicit IntegerOverflowVisitor(clang::ASTContext* context, Reporter& reporter)
        : context_(context), reporter_(reporter) {}

    bool VisitBinaryOperator(clang::BinaryOperator* op);
    bool VisitCStyleCastExpr(clang::CStyleCastExpr* cast);
    bool VisitCXXStaticCastExpr(clang::CXXStaticCastExpr* cast);

private:
    void checkArithmeticOverflow(clang::BinaryOperator* op);
    void checkNarrowingConversion(clang::Expr* expr, clang::QualType targetType, clang::SourceLocation loc);
    bool isIntegerType(clang::QualType type);
    unsigned getIntegerBitWidth(clang::QualType type);

    clang::ASTContext* context_;
    Reporter& reporter_;
};

class IntegerOverflowRule : public Rule {
public:
    std::string getRuleId() const override {
        return "INTEGER-OVERFLOW-001";
    }

    std::string getRuleName() const override {
        return "Integer Overflow Detection";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override {
        IntegerOverflowVisitor visitor(context, reporter);
        visitor.TraverseDecl(context->getTranslationUnitDecl());
    }
};

} // namespace cpp_review
