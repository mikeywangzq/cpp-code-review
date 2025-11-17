#include "rules/uninitialized_var_rule.h"
#include <clang/AST/Type.h>

namespace cpp_review {

bool UninitializedVarVisitor::isBuiltinType(clang::QualType type) {
    return type->isBuiltinType() || type->isPointerType();
}

bool UninitializedVarVisitor::shouldCheckVariable(clang::VarDecl* decl) {
    // Don't check static or extern variables
    if (decl->isStaticLocal() || decl->hasExternalStorage()) {
        return false;
    }

    // Don't check reference types
    if (decl->getType()->isReferenceType()) {
        return false;
    }

    // Don't check class types with default constructors (they're initialized)
    if (auto* recordType = decl->getType()->getAsCXXRecordDecl()) {
        if (recordType->hasDefaultConstructor()) {
            return false;
        }
    }

    return true;
}

bool UninitializedVarVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!shouldCheckVariable(decl)) {
        return true;
    }

    // Check if the variable is uninitialized
    if (!decl->hasInit() && isBuiltinType(decl->getType())) {
        Issue issue;
        issue.file_path = getFileName(decl->getLocation());
        issue.line = getLine(decl->getLocation());
        issue.column = getColumn(decl->getLocation());
        issue.severity = Severity::HIGH;
        issue.rule_id = "UNINIT-VAR-001";

        std::string varName = decl->getNameAsString();
        std::string typeName = decl->getType().getAsString();

        issue.description = "Variable '" + varName + "' of type '" + typeName +
                          "' is declared but not initialized. Using uninitialized variables leads to undefined behavior";
        issue.suggestion = "Initialize the variable at declaration, e.g., '" + typeName + " " +
                         varName + " = <value>;' or use '{}' for zero-initialization: '" +
                         typeName + " " + varName + "{};'";

        // Get code snippet
        clang::SourceRange range = decl->getSourceRange();
        issue.code_snippet = getSourceText(range);

        reporter_.addIssue(issue);
    }

    return true;
}

void UninitializedVarRule::check(clang::ASTContext* context, Reporter& reporter) {
    UninitializedVarVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
