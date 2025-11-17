#include "rules/use_after_free_rule.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"

namespace cpp_review {

bool UseAfterFreeVisitor::VisitFunctionDecl(clang::FunctionDecl* func) {
    if (!func || !func->hasBody()) {
        return true;
    }

    // Clear state for each function
    deletedPointers_.clear();
    deleteLocations_.clear();

    // Analyze function body
    analyzeFunctionBody(func->getBody());

    return true;
}

void UseAfterFreeVisitor::analyzeFunctionBody(clang::Stmt* body) {
    if (!body) return;

    // Traverse all statements in the function body
    for (auto it = body->child_begin(); it != body->child_end(); ++it) {
        if (*it) {
            analyzeStmt(*it);
        }
    }
}

void UseAfterFreeVisitor::analyzeStmt(clang::Stmt* stmt) {
    if (!stmt) return;

    // Check for delete expressions
    if (auto* deleteExpr = llvm::dyn_cast<clang::CXXDeleteExpr>(stmt)) {
        checkDeleteExpr(deleteExpr);
    }
    // Check for pointer usage (dereference, member access, array subscript)
    else if (auto* unaryOp = llvm::dyn_cast<clang::UnaryOperator>(stmt)) {
        if (unaryOp->getOpcode() == clang::UO_Deref) {
            checkPointerUsage(unaryOp->getSubExpr());
        }
    }
    else if (auto* memberExpr = llvm::dyn_cast<clang::MemberExpr>(stmt)) {
        if (memberExpr->isArrow()) {
            checkPointerUsage(memberExpr->getBase());
        }
    }
    else if (auto* arraySubscript = llvm::dyn_cast<clang::ArraySubscriptExpr>(stmt)) {
        checkPointerUsage(arraySubscript->getBase());
    }
    else if (auto* callExpr = llvm::dyn_cast<clang::CallExpr>(stmt)) {
        // Check function arguments for pointer usage
        for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
            if (auto* arg = callExpr->getArg(i)) {
                if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(arg->IgnoreImpCasts())) {
                    if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                        if (isPointerDeleted(varDecl)) {
                            Issue issue;
                            issue.file_path = context_->getSourceManager().getFilename(arg->getExprLoc()).str();
                            issue.line = context_->getSourceManager().getSpellingLineNumber(arg->getExprLoc());
                            issue.column = context_->getSourceManager().getSpellingColumnNumber(arg->getExprLoc());
                            issue.severity = Severity::CRITICAL;
                            issue.rule_id = "USE-AFTER-FREE-001";

                            issue.description = "Use-after-free: Pointer '" + varDecl->getNameAsString() +
                                              "' is used after being deleted.";

                            issue.suggestion = "Do not use a pointer after deleting it:\n"
                                             "  - Set pointer to nullptr after delete\n"
                                             "  - Use smart pointers (std::unique_ptr, std::shared_ptr)\n"
                                             "  - Restructure code to avoid using deleted pointers";

                            reporter_.addIssue(issue);
                        }
                    }
                }
            }
        }
    }

    // Recursively analyze child statements
    for (auto it = stmt->child_begin(); it != stmt->child_end(); ++it) {
        if (*it) {
            analyzeStmt(*it);
        }
    }
}

void UseAfterFreeVisitor::checkDeleteExpr(clang::CXXDeleteExpr* deleteExpr) {
    if (!deleteExpr) return;

    clang::Expr* argument = deleteExpr->getArgument();
    if (!argument) return;

    const clang::ValueDecl* decl = getReferencedDecl(argument);
    if (decl) {
        deletedPointers_.insert(decl);
        deleteLocations_[decl] = deleteExpr->getBeginLoc();
    }
}

void UseAfterFreeVisitor::checkPointerUsage(clang::Expr* expr) {
    if (!expr) return;

    const clang::ValueDecl* decl = getReferencedDecl(expr);
    if (!decl) return;

    if (isPointerDeleted(decl)) {
        Issue issue;
        issue.file_path = context_->getSourceManager().getFilename(expr->getExprLoc()).str();
        issue.line = context_->getSourceManager().getSpellingLineNumber(expr->getExprLoc());
        issue.column = context_->getSourceManager().getSpellingColumnNumber(expr->getExprLoc());
        issue.severity = Severity::CRITICAL;
        issue.rule_id = "USE-AFTER-FREE-001";

        issue.description = "Use-after-free detected: Dereferencing pointer '" +
                          decl->getNameAsString() + "' after it has been deleted.";

        auto deleteLoc = deleteLocations_[decl];
        unsigned deleteLine = context_->getSourceManager().getSpellingLineNumber(deleteLoc);

        issue.suggestion = "Pointer was deleted at line " + std::to_string(deleteLine) + ". "
                         "Do not use pointers after deletion:\n"
                         "  - Set pointer to nullptr after delete: delete ptr; ptr = nullptr;\n"
                         "  - Use smart pointers that automatically manage lifetime\n"
                         "  - Add a check: if (ptr != nullptr) { use ptr }";

        reporter_.addIssue(issue);
    }
}

bool UseAfterFreeVisitor::isPointerDeleted(const clang::ValueDecl* decl) {
    return deletedPointers_.find(decl) != deletedPointers_.end();
}

const clang::ValueDecl* UseAfterFreeVisitor::getReferencedDecl(clang::Expr* expr) {
    if (!expr) return nullptr;

    // Strip implicit casts
    expr = expr->IgnoreImpCasts();

    if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
        if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
            return varDecl;
        }
    }

    return nullptr;
}

} // namespace cpp_review
