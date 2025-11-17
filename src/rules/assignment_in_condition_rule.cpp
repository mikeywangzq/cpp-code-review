#include "rules/assignment_in_condition_rule.h"
#include <clang/AST/OperationKinds.h>
#include <clang/AST/ParentMapContext.h>

namespace cpp_review {

bool AssignmentInConditionVisitor::hasAssignment(clang::Expr* expr) {
    if (!expr) return false;

    // Remove implicit casts and parentheses
    expr = expr->IgnoreParenImpCasts();

    // Check for binary assignment operator
    if (auto* binOp = llvm::dyn_cast<clang::BinaryOperator>(expr)) {
        if (binOp->isAssignmentOp()) {
            // Exclude compound assignments like +=, -=, etc. which are more intentional
            return binOp->getOpcode() == clang::BO_Assign;
        }
    }

    return false;
}

void AssignmentInConditionVisitor::checkCondition(clang::Expr* cond, clang::SourceLocation loc) {
    if (!cond) return;

    if (hasAssignment(cond)) {
        Issue issue;
        issue.file_path = getFileName(loc);
        issue.line = getLine(loc);
        issue.column = getColumn(loc);
        issue.severity = Severity::HIGH;
        issue.rule_id = "ASSIGN-COND-001";
        issue.description = "Assignment operator (=) used in conditional expression. This is likely a bug - did you mean to use comparison operator (==)?";
        issue.suggestion = "Replace '=' with '==' for comparison. If assignment was intentional, make it explicit by adding extra parentheses: if ((a = b))";

        // Get code snippet
        clang::SourceRange range = cond->getSourceRange();
        issue.code_snippet = getSourceText(range);

        reporter_.addIssue(issue);
    }
}

bool AssignmentInConditionVisitor::VisitIfStmt(clang::IfStmt* stmt) {
    checkCondition(stmt->getCond(), stmt->getIfLoc());
    return true;
}

bool AssignmentInConditionVisitor::VisitWhileStmt(clang::WhileStmt* stmt) {
    checkCondition(stmt->getCond(), stmt->getWhileLoc());
    return true;
}

bool AssignmentInConditionVisitor::VisitForStmt(clang::ForStmt* stmt) {
    checkCondition(stmt->getCond(), stmt->getForLoc());
    return true;
}

void AssignmentInConditionRule::check(clang::ASTContext* context, Reporter& reporter) {
    AssignmentInConditionVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
