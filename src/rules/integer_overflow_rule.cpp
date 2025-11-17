#include "rules/integer_overflow_rule.h"
#include "clang/AST/Expr.h"
#include <algorithm>

namespace cpp_review {

bool IntegerOverflowVisitor::isIntegerType(clang::QualType type) {
    type = type.getCanonicalType();
    return type->isIntegerType() && !type->isBooleanType() && !type->isCharType();
}

unsigned IntegerOverflowVisitor::getIntegerBitWidth(clang::QualType type) {
    type = type.getCanonicalType();
    if (const auto* builtinType = type->getAs<clang::BuiltinType>()) {
        return context_->getTypeSize(builtinType);
    }
    return 0;
}

bool IntegerOverflowVisitor::VisitBinaryOperator(clang::BinaryOperator* op) {
    if (!op) return true;

    checkArithmeticOverflow(op);
    return true;
}

void IntegerOverflowVisitor::checkArithmeticOverflow(clang::BinaryOperator* op) {
    // Check for potential overflow in arithmetic operations
    if (!op->getType()->isIntegerType()) {
        return;
    }

    clang::BinaryOperator::Opcode opcode = op->getOpcode();

    // Only check operations that can overflow
    if (opcode != clang::BO_Add &&
        opcode != clang::BO_Mul &&
        opcode != clang::BO_Sub &&
        opcode != clang::BO_AddAssign &&
        opcode != clang::BO_MulAssign &&
        opcode != clang::BO_SubAssign) {
        return;
    }

    clang::Expr* lhs = op->getLHS();
    clang::Expr* rhs = op->getRHS();

    if (!lhs || !rhs) return;

    clang::QualType lhsType = lhs->getType();
    clang::QualType rhsType = rhs->getType();

    // Skip if either operand is not an integer type
    if (!isIntegerType(lhsType) || !isIntegerType(rhsType)) {
        return;
    }

    // Check for small integer types that are more prone to overflow
    unsigned lhsBits = getIntegerBitWidth(lhsType);
    unsigned rhsBits = getIntegerBitWidth(rhsType);
    unsigned maxBits = std::max(lhsBits, rhsBits);

    // Report issues for 8-bit and 16-bit integers, and 32-bit for multiplication
    bool shouldReport = false;
    std::string typeDescription;

    if (maxBits <= 16) {
        shouldReport = true;
        typeDescription = maxBits == 8 ? "8-bit" : "16-bit";
    } else if (maxBits == 32 && (opcode == clang::BO_Mul || opcode == clang::BO_MulAssign)) {
        shouldReport = true;
        typeDescription = "32-bit";
    }

    if (shouldReport) {
        Issue issue;
        issue.file_path = context_->getSourceManager().getFilename(op->getExprLoc()).str();
        issue.line = context_->getSourceManager().getSpellingLineNumber(op->getExprLoc());
        issue.column = context_->getSourceManager().getSpellingColumnNumber(op->getExprLoc());
        issue.severity = maxBits <= 16 ? Severity::HIGH : Severity::MEDIUM;
        issue.rule_id = "INTEGER-OVERFLOW-001";

        std::string opStr;
        switch (opcode) {
            case clang::BO_Add:
            case clang::BO_AddAssign:
                opStr = "addition";
                break;
            case clang::BO_Mul:
            case clang::BO_MulAssign:
                opStr = "multiplication";
                break;
            case clang::BO_Sub:
            case clang::BO_SubAssign:
                opStr = "subtraction";
                break;
            default:
                opStr = "arithmetic operation";
        }

        issue.description = "Potential integer overflow in " + opStr + " with " + typeDescription +
                          " integer types. Consider using larger types or overflow checking.";

        issue.suggestion = "Use larger integer types (e.g., int64_t, long long) or add overflow checks:\n"
                         "  - For C++: Use std::numeric_limits to check bounds\n"
                         "  - For GCC/Clang: Use __builtin_add_overflow() family of functions\n"
                         "  - Consider using safe integer libraries";

        reporter_.addIssue(issue);
    }
}

bool IntegerOverflowVisitor::VisitCStyleCastExpr(clang::CStyleCastExpr* cast) {
    if (!cast) return true;

    clang::QualType targetType = cast->getType();
    clang::Expr* subExpr = cast->getSubExpr();

    if (subExpr) {
        checkNarrowingConversion(subExpr, targetType, cast->getExprLoc());
    }

    return true;
}

bool IntegerOverflowVisitor::VisitCXXStaticCastExpr(clang::CXXStaticCastExpr* cast) {
    if (!cast) return true;

    clang::QualType targetType = cast->getType();
    clang::Expr* subExpr = cast->getSubExpr();

    if (subExpr) {
        checkNarrowingConversion(subExpr, targetType, cast->getExprLoc());
    }

    return true;
}

void IntegerOverflowVisitor::checkNarrowingConversion(clang::Expr* expr, clang::QualType targetType, clang::SourceLocation loc) {
    if (!expr) return;

    clang::QualType sourceType = expr->getType();

    // Check if both are integer types
    if (!isIntegerType(sourceType) || !isIntegerType(targetType)) {
        return;
    }

    unsigned sourceBits = getIntegerBitWidth(sourceType);
    unsigned targetBits = getIntegerBitWidth(targetType);

    // Check for narrowing conversion
    if (sourceBits > targetBits) {
        Issue issue;
        issue.file_path = context_->getSourceManager().getFilename(loc).str();
        issue.line = context_->getSourceManager().getSpellingLineNumber(loc);
        issue.column = context_->getSourceManager().getSpellingColumnNumber(loc);
        issue.severity = Severity::MEDIUM;
        issue.rule_id = "INTEGER-OVERFLOW-001";

        issue.description = "Narrowing integer conversion from " + std::to_string(sourceBits) +
                          "-bit to " + std::to_string(targetBits) + "-bit type may truncate data.";

        issue.suggestion = "Ensure the value fits in the target type:\n"
                         "  - Add range checking before conversion\n"
                         "  - Use static_assert with std::numeric_limits for compile-time checks\n"
                         "  - Consider using a wider type if possible";

        reporter_.addIssue(issue);
    }
}

} // namespace cpp_review
