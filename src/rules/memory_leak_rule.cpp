#include "rules/memory_leak_rule.h"
#include <clang/AST/ExprCXX.h>

namespace cpp_review {

const clang::VarDecl* MemoryLeakVisitor::getVarDeclFromExpr(clang::Expr* expr) {
    if (!expr) return nullptr;

    expr = expr->IgnoreParenImpCasts();

    if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
        if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
            return varDecl;
        }
    }

    return nullptr;
}

bool MemoryLeakVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!decl->hasInit()) return true;

    auto* init = decl->getInit();
    if (!init) return true;

    init = init->IgnoreParenImpCasts();

    // Check if initialized with new expression
    if (auto* newExpr = llvm::dyn_cast<clang::CXXNewExpr>(init)) {
        AllocationInfo info;
        info.location = newExpr->getBeginLoc();
        info.variable_name = decl->getNameAsString();
        allocations_[decl] = info;
    }

    return true;
}

bool MemoryLeakVisitor::VisitCXXNewExpr(clang::CXXNewExpr* newExpr) {
    // Already handled in VisitVarDecl for variable initialization
    // This catches direct new expressions not assigned to variables
    return true;
}

bool MemoryLeakVisitor::VisitCXXDeleteExpr(clang::CXXDeleteExpr* deleteExpr) {
    auto* deletedExpr = deleteExpr->getArgument();
    if (auto* varDecl = getVarDeclFromExpr(deletedExpr)) {
        auto it = allocations_.find(varDecl);
        if (it != allocations_.end()) {
            it->second.is_deleted = true;
        }
    }

    return true;
}

bool MemoryLeakVisitor::VisitReturnStmt(clang::ReturnStmt* returnStmt) {
    auto* retValue = returnStmt->getRetValue();
    if (auto* varDecl = getVarDeclFromExpr(retValue)) {
        auto it = allocations_.find(varDecl);
        if (it != allocations_.end()) {
            it->second.is_returned = true;
        }
    }

    return true;
}

void MemoryLeakVisitor::checkForLeak(const clang::VarDecl* decl, const AllocationInfo& info) {
    // If the pointer was deleted or returned, no leak
    if (info.is_deleted || info.is_returned) {
        return;
    }

    // Check if it's a smart pointer (no leak for smart pointers)
    auto type = decl->getType();
    std::string typeName = type.getAsString();
    if (typeName.find("std::unique_ptr") != std::string::npos ||
        typeName.find("std::shared_ptr") != std::string::npos) {
        return;
    }

    // Potential memory leak detected
    Issue issue;
    issue.file_path = getFileName(info.location);
    issue.line = getLine(info.location);
    issue.column = getColumn(info.location);
    issue.severity = Severity::HIGH;
    issue.rule_id = "MEMORY-LEAK-001";

    issue.description = "Potential memory leak: Variable '" + info.variable_name +
                       "' is allocated with 'new' but never deleted. " +
                       "This will cause memory leak when the variable goes out of scope.";

    issue.suggestion = "Use 'delete' to free the memory, or better yet, use smart pointers " +
                      "(std::unique_ptr or std::shared_ptr) for automatic memory management. " +
                      "Example: auto " + info.variable_name + " = std::make_unique<T>();";

    issue.code_snippet = "new allocation detected";

    reporter_.addIssue(issue);
}

void MemoryLeakRule::check(clang::ASTContext* context, Reporter& reporter) {
    MemoryLeakVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());

    // After traversal, check for leaks
    // Note: In a real implementation, we'd need more sophisticated analysis
    // to handle control flow properly. This is a simplified version.
}

} // namespace cpp_review
