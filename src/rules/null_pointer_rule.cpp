#include "rules/null_pointer_rule.h"
#include <clang/AST/ASTContext.h>
#include <clang/Lex/Lexer.h>

namespace cpp_review {

bool NullPointerVisitor::isNullPointer(clang::Expr* expr) {
    if (!expr) return false;

    expr = expr->IgnoreParenCasts();

    // Check for nullptr
    if (llvm::isa<clang::CXXNullPtrLiteralExpr>(expr)) {
        return true;
    }

    // Check for NULL macro or 0
    if (auto* intLiteral = llvm::dyn_cast<clang::IntegerLiteral>(expr)) {
        return intLiteral->getValue() == 0;
    }

    // Check for GNU null
    if (auto* gnuNull = llvm::dyn_cast<clang::GNUNullExpr>(expr)) {
        return true;
    }

    return false;
}

bool NullPointerVisitor::isPointerType(clang::Expr* expr) {
    if (!expr) return false;
    return expr->getType()->isPointerType();
}

void NullPointerVisitor::checkForNullDereference(clang::Expr* expr, clang::SourceLocation loc) {
    if (!expr || !isPointerType(expr)) return;

    if (isNullPointer(expr)) {
        Issue issue;
        issue.file_path = getFileName(loc);
        issue.line = getLine(loc);
        issue.column = getColumn(loc);
        issue.severity = Severity::CRITICAL;
        issue.rule_id = "NULL-PTR-001";
        issue.description = "Dereferencing a null pointer will cause undefined behavior and likely crash";
        issue.suggestion = "Check for null before dereferencing, or use smart pointers (std::unique_ptr, std::shared_ptr) which provide better safety guarantees";

        // Get code snippet
        clang::SourceRange range = expr->getSourceRange();
        issue.code_snippet = getSourceText(range);

        reporter_.addIssue(issue);
    }
}

bool NullPointerVisitor::VisitUnaryOperator(clang::UnaryOperator* op) {
    if (op->getOpcode() == clang::UO_Deref) {
        checkForNullDereference(op->getSubExpr(), op->getOperatorLoc());
    }
    return true;
}

bool NullPointerVisitor::VisitMemberExpr(clang::MemberExpr* expr) {
    if (expr->isArrow()) {
        checkForNullDereference(expr->getBase(), expr->getOperatorLoc());
    }
    return true;
}

bool NullPointerVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr) {
    checkForNullDereference(expr->getBase(), expr->getBeginLoc());
    return true;
}

bool NullPointerVisitor::VisitBinaryOperator(clang::BinaryOperator* op) {
    // Check assignments to see if we're assigning null
    return true;
}

std::string RuleVisitor::getFileName(clang::SourceLocation loc) const {
    if (loc.isInvalid()) return "<unknown>";
    return context_->getSourceManager().getFilename(loc).str();
}

unsigned RuleVisitor::getLine(clang::SourceLocation loc) const {
    if (loc.isInvalid()) return 0;
    return context_->getSourceManager().getSpellingLineNumber(loc);
}

unsigned RuleVisitor::getColumn(clang::SourceLocation loc) const {
    if (loc.isInvalid()) return 0;
    return context_->getSourceManager().getSpellingColumnNumber(loc);
}

std::string RuleVisitor::getSourceText(clang::SourceRange range) const {
    if (range.isInvalid()) return "";

    clang::SourceManager& sm = context_->getSourceManager();
    clang::LangOptions lo;

    clang::CharSourceRange charRange = clang::CharSourceRange::getTokenRange(range);
    llvm::StringRef text = clang::Lexer::getSourceText(charRange, sm, lo);

    return text.str();
}

void NullPointerRule::check(clang::ASTContext* context, Reporter& reporter) {
    NullPointerVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
