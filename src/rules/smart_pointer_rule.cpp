#include "rules/smart_pointer_rule.h"
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Type.h>

namespace cpp_review {

// Helper function to robustly check if a type is a smart pointer
static bool isSmartPointerType(clang::QualType type) {
    type = type.getCanonicalType().getUnqualifiedType();

    // Check if it's a template specialization
    if (const auto* templateType = type->getAs<clang::TemplateSpecializationType>()) {
        if (auto* templateDecl = templateType->getTemplateName().getAsTemplateDecl()) {
            std::string qualifiedName = templateDecl->getQualifiedNameAsString();
            // Check for standard smart pointers
            return qualifiedName == "std::unique_ptr" ||
                   qualifiedName == "std::shared_ptr" ||
                   qualifiedName == "std::weak_ptr";
        }
    }

    // Fallback to string matching for edge cases (e.g., type aliases)
    std::string typeName = type.getAsString();
    return typeName.find("std::unique_ptr") != std::string::npos ||
           typeName.find("std::shared_ptr") != std::string::npos ||
           typeName.find("std::weak_ptr") != std::string::npos;
}


bool SmartPointerVisitor::isRawPointerWithNew(clang::VarDecl* decl) {
    // Check if this is a raw pointer type
    if (!decl->getType()->isPointerType()) {
        return false;
    }

    // Check if it's not already a smart pointer
    // Use robust type-based detection instead of fragile string matching
    if (isSmartPointerType(decl->getType())) {
        return false;
    }

    // Check if initialized with new
    if (!decl->hasInit()) {
        return false;
    }

    auto* init = decl->getInit()->IgnoreParenImpCasts();
    return llvm::isa<clang::CXXNewExpr>(init);
}

bool SmartPointerVisitor::shouldUseUniquePtr(clang::VarDecl* decl) {
    // Unique ownership pattern - most common case
    return true;
}

bool SmartPointerVisitor::shouldUseSharedPtr(clang::VarDecl* decl) {
    // Would need more analysis to determine if shared ownership is needed
    return false;
}

bool SmartPointerVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!isRawPointerWithNew(decl)) {
        return true;
    }

    Issue issue;
    issue.file_path = getFileName(decl->getLocation());
    issue.line = getLine(decl->getLocation());
    issue.column = getColumn(decl->getLocation());
    issue.severity = Severity::SUGGESTION;
    issue.rule_id = "SMART-PTR-001";

    std::string varName = decl->getNameAsString();
    clang::QualType pointeeType = decl->getType()->getPointeeType();
    std::string typeName = pointeeType.getAsString();

    issue.description = "Consider using smart pointers instead of raw pointer '" + varName +
                       "'. Smart pointers provide automatic memory management and prevent memory leaks.";

    if (shouldUseUniquePtr(decl)) {
        issue.suggestion = "Replace with std::unique_ptr for exclusive ownership:\n" +
                          std::string("  auto ") + varName + " = std::make_unique<" + typeName + ">();\n" +
                          "Or if constructing with parameters:\n" +
                          "  auto " + varName + " = std::make_unique<" + typeName + ">(args...);";
    } else if (shouldUseSharedPtr(decl)) {
        issue.suggestion = "Replace with std::shared_ptr for shared ownership:\n" +
                          std::string("  auto ") + varName + " = std::make_shared<" + typeName + ">();";
    }

    issue.code_snippet = getSourceText(decl->getSourceRange());

    reporter_.addIssue(issue);

    return true;
}

bool SmartPointerVisitor::VisitCXXNewExpr(clang::CXXNewExpr* newExpr) {
    // Additional checks for new expressions can be added here
    return true;
}

void SmartPointerRule::check(clang::ASTContext* context, Reporter& reporter) {
    SmartPointerVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
