cmake_minimum_required(VERSION 3.15)
project(cpp-code-review VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optional: Enable CURL for API support
option(ENABLE_CURL "Enable CURL for LLM API support" OFF)

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
    src/llm/llm_enhancer.cpp
    src/git/git_integration.cpp
    src/fixer/auto_fixer.cpp
    src/analysis/taint_analysis.cpp
)

# Optional API client sources
if(ENABLE_CURL)
    list(APPEND SOURCES src/llm/api_client.cpp)
    add_definitions(-DENABLE_CURL)
endif()

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

# Optional CURL library for API support
if(ENABLE_CURL)
    find_package(CURL REQUIRED)
    target_link_libraries(cpp-agent ${CURL_LIBRARIES})
    target_include_directories(cpp-agent PRIVATE ${CURL_INCLUDE_DIRS})
endif()

# Installation
install(TARGETS cpp-agent DESTINATION bin)
