cmake_minimum_required(VERSION 3.15)
project(cpp-code-review VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add LLVM definitions
add_definitions(${LLVM_DEFINITIONS})

# Source files
set(SOURCES
    src/main.cpp
    src/parser/ast_parser.cpp
    src/rules/rule_engine.cpp
    src/rules/null_pointer_rule.cpp
    src/rules/uninitialized_var_rule.cpp
    src/rules/assignment_in_condition_rule.cpp
    src/rules/unsafe_c_functions_rule.cpp
    src/rules/memory_leak_rule.cpp
    src/rules/smart_pointer_rule.cpp
    src/rules/loop_copy_rule.cpp
    src/rules/integer_overflow_rule.cpp
    src/rules/use_after_free_rule.cpp
    src/rules/buffer_overflow_rule.cpp
    src/report/reporter.cpp
    src/report/html_reporter.cpp
    src/config/config.cpp
    src/cli/cli.cpp
)

# Create executable
add_executable(cpp-agent ${SOURCES})

# Link against LLVM and Clang libraries
target_link_libraries(cpp-agent
    clangTooling
    clangFrontend
    clangDriver
    clangSerialization
    clangCodeGen
    clangParse
    clangSema
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangAnalysis
    clangRewrite
    clangRewriteFrontend
    clangEdit
    clangAST
    clangASTMatchers
    clangLex
    clangBasic
)

# LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    support
    core
    irreader
)
target_link_libraries(cpp-agent ${llvm_libs})

# Installation
install(TARGETS cpp-agent DESTINATION bin)
