#include "rules/buffer_overflow_rule.h"
#include "clang/AST/Expr.h"

namespace cpp_review {

bool BufferOverflowVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!decl) return true;

    clang::QualType type = decl->getType();

    // Check if it's a constant array type
    if (const auto* arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr())) {
        uint64_t size = arrayType->getSize().getZExtValue();
        arraySizes_[decl] = size;
    }

    return true;
}

bool BufferOverflowVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr) {
    if (!expr) return true;

    clang::Expr* base = expr->getBase()->IgnoreImpCasts();
    clang::Expr* index = expr->getIdx();

    if (!base || !index) return true;

    // Try to get the array declaration
    const clang::VarDecl* arrayDecl = nullptr;

    if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(base)) {
        arrayDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }

    if (!arrayDecl) return true;

    // Check if we know the array size
    auto it = arraySizes_.find(arrayDecl);
    if (it == arraySizes_.end()) {
        // Array size unknown, check for negative indices
        int64_t indexValue;
        if (tryGetConstantIndex(index, indexValue)) {
            if (indexValue < 0) {
                Issue issue;
                issue.file_path = context_->getSourceManager().getFilename(expr->getExprLoc()).str();
                issue.line = context_->getSourceManager().getSpellingLineNumber(expr->getExprLoc());
                issue.column = context_->getSourceManager().getSpellingColumnNumber(expr->getExprLoc());
                issue.severity = Severity::CRITICAL;
                issue.rule_id = "BUFFER-OVERFLOW-001";

                issue.description = "Array access with negative index " + std::to_string(indexValue) +
                                  " will cause buffer underflow.";

                issue.suggestion = "Use non-negative array indices:\n"
                                 "  - Ensure index >= 0 before array access\n"
                                 "  - Use unsigned types for array indices\n"
                                 "  - Consider using std::vector with at() for bounds checking";

                reporter_.addIssue(issue);
            }
        }
        return true;
    }

    uint64_t arraySize = it->second;

    // Try to get constant index value
    int64_t indexValue;
    if (tryGetConstantIndex(index, indexValue)) {
        // Check for out-of-bounds access
        // Note: The check for "indexValue < 0" ensures indexValue is non-negative before
        // the unsigned cast, preventing signed/unsigned comparison issues. This is safe.
        if (indexValue < 0 || static_cast<uint64_t>(indexValue) >= arraySize) {
            Issue issue;
            issue.file_path = context_->getSourceManager().getFilename(expr->getExprLoc()).str();
            issue.line = context_->getSourceManager().getSpellingLineNumber(expr->getExprLoc());
            issue.column = context_->getSourceManager().getSpellingColumnNumber(expr->getExprLoc());
            issue.severity = Severity::CRITICAL;
            issue.rule_id = "BUFFER-OVERFLOW-001";

            std::string direction = indexValue < 0 ? "underflow" : "overflow";

            issue.description = "Buffer " + direction + ": Array '" + arrayDecl->getNameAsString() +
                              "' has size " + std::to_string(arraySize) + " but accessed with index " +
                              std::to_string(indexValue) + ".";

            issue.suggestion = "Ensure array index is within valid range [0, " +
                             std::to_string(arraySize - 1) + "]:\n"
                             "  - Add bounds checking: if (index < size) { array[index] }\n"
                             "  - Use std::array or std::vector with at() for automatic bounds checking\n"
                             "  - Fix the constant index to be within valid range";

            reporter_.addIssue(issue);
        }
    } else {
        // Non-constant index - suggest bounds checking
        // Only report for small arrays where overflow is more likely
        if (arraySize <= 10) {
            Issue issue;
            issue.file_path = context_->getSourceManager().getFilename(expr->getExprLoc()).str();
            issue.line = context_->getSourceManager().getSpellingLineNumber(expr->getExprLoc());
            issue.column = context_->getSourceManager().getSpellingColumnNumber(expr->getExprLoc());
            issue.severity = Severity::LOW;
            issue.rule_id = "BUFFER-OVERFLOW-001";

            issue.description = "Array '" + arrayDecl->getNameAsString() + "' accessed with non-constant index. "
                              "Array has size " + std::to_string(arraySize) + ". Consider adding bounds checking.";

            issue.suggestion = "Add bounds checking for dynamic array access:\n"
                             "  - if (index >= 0 && index < " + std::to_string(arraySize) + ") { array[index] }\n"
                             "  - Use std::array::at() or std::vector::at() for automatic bounds checking\n"
                             "  - Use assertions: assert(index >= 0 && index < size)";

            reporter_.addIssue(issue);
        }
    }

    return true;
}

uint64_t BufferOverflowVisitor::getArraySize(const clang::VarDecl* decl) {
    auto it = arraySizes_.find(decl);
    if (it != arraySizes_.end()) {
        return it->second;
    }
    return 0;
}

bool BufferOverflowVisitor::tryGetConstantIndex(clang::Expr* expr, int64_t& value) {
    if (!expr) return false;

    expr = expr->IgnoreImpCasts();

    // Try to evaluate as constant expression
    clang::Expr::EvalResult result;
    if (expr->EvaluateAsInt(result, *context_)) {
        value = result.Val.getInt().getSExtValue();
        return true;
    }

    return false;
}

} // namespace cpp_review
