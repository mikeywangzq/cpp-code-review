#include "rules/loop_copy_rule.h"
#include <clang/AST/Type.h>
#include <clang/AST/DeclCXX.h>

namespace cpp_review {

bool LoopCopyVisitor::isContainerType(clang::QualType type) {
    std::string typeName = type.getAsString();
    return typeName.find("std::vector") != std::string::npos ||
           typeName.find("std::string") != std::string::npos ||
           typeName.find("std::map") != std::string::npos ||
           typeName.find("std::set") != std::string::npos ||
           typeName.find("std::list") != std::string::npos ||
           typeName.find("std::deque") != std::string::npos;
}

bool LoopCopyVisitor::isClassType(clang::QualType type) {
    return type->isRecordType() || type->isCXXClassType();
}

bool LoopCopyVisitor::isExpensiveCopy(clang::VarDecl* decl) {
    if (!decl->hasInit()) {
        return false;
    }

    clang::QualType type = decl->getType();

    // Remove const and reference qualifiers for analysis
    type = type.getNonReferenceType();
    type = type.getUnqualifiedType();

    // Check if it's a reference or pointer (no copy)
    if (decl->getType()->isReferenceType() || decl->getType()->isPointerType()) {
        return false;
    }

    // Check if it's a container type (expensive to copy)
    if (isContainerType(type)) {
        return true;
    }

    // Check if it's a large class type
    if (isClassType(type)) {
        if (auto* recordDecl = type->getAsCXXRecordDecl()) {
            // Simplified heuristic: if it has multiple members, consider it expensive
            if (std::distance(recordDecl->field_begin(), recordDecl->field_end()) > 2) {
                return true;
            }
        }
    }

    return false;
}

bool LoopCopyVisitor::LoopBodyVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (parent_->isExpensiveCopy(decl)) {
        Issue issue;
        issue.file_path = parent_->getFileName(decl->getLocation());
        issue.line = parent_->getLine(decl->getLocation());
        issue.column = parent_->getColumn(decl->getLocation());
        issue.severity = Severity::MEDIUM;
        issue.rule_id = "LOOP-COPY-001";

        std::string varName = decl->getNameAsString();
        std::string typeName = decl->getType().getAsString();

        issue.description = "Expensive copy operation in loop: Variable '" + varName +
                           "' of type '" + typeName + "' is being copied. " +
                           "This can significantly impact performance in tight loops.";

        issue.suggestion = "Use const reference to avoid copying:\n" +
                          std::string("  const ") + typeName + "& " + varName + " = ...;\n" +
                          "Or use std::move if the original value is no longer needed:\n" +
                          "  " + typeName + " " + varName + " = std::move(...);";

        issue.code_snippet = parent_->getSourceText(decl->getSourceRange());

        parent_->reporter_.addIssue(issue);
    }

    return true;
}

void LoopCopyVisitor::checkLoopBody(clang::Stmt* body, clang::SourceLocation loopLoc) {
    if (!body) return;

    LoopBodyVisitor bodyVisitor(this);
    bodyVisitor.TraverseStmt(body);
}

bool LoopCopyVisitor::VisitForStmt(clang::ForStmt* forStmt) {
    checkLoopBody(forStmt->getBody(), forStmt->getForLoc());
    return true;
}

bool LoopCopyVisitor::VisitCXXForRangeStmt(clang::CXXForRangeStmt* rangeStmt) {
    // Check the loop variable itself
    if (auto* loopVar = rangeStmt->getLoopVariable()) {
        if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(loopVar)) {
            if (!varDecl->getType()->isReferenceType() &&
                !varDecl->getType()->isPointerType()) {

                clang::QualType type = varDecl->getType();
                if (isContainerType(type) || isClassType(type)) {
                    Issue issue;
                    issue.file_path = getFileName(varDecl->getLocation());
                    issue.line = getLine(varDecl->getLocation());
                    issue.column = getColumn(varDecl->getLocation());
                    issue.severity = Severity::MEDIUM;
                    issue.rule_id = "LOOP-COPY-001";

                    std::string varName = varDecl->getNameAsString();
                    std::string typeName = type.getAsString();

                    issue.description = "Range-based for loop is copying elements. " +
                                       std::string("Each iteration copies the entire ") + typeName + ".";

                    issue.suggestion = "Use const reference in range-based for loop:\n" +
                                      std::string("  for (const auto& ") + varName + " : container) { ... }\n" +
                                      "Or use reference if you need to modify:\n" +
                                      "  for (auto& " + varName + " : container) { ... }";

                    issue.code_snippet = getSourceText(rangeStmt->getSourceRange());

                    reporter_.addIssue(issue);
                }
            }
        }
    }

    checkLoopBody(rangeStmt->getBody(), rangeStmt->getForLoc());
    return true;
}

bool LoopCopyVisitor::VisitWhileStmt(clang::WhileStmt* whileStmt) {
    checkLoopBody(whileStmt->getBody(), whileStmt->getWhileLoc());
    return true;
}

void LoopCopyRule::check(clang::ASTContext* context, Reporter& reporter) {
    LoopCopyVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
