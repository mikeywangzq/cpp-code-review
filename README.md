# C++ 智能代码审查 Agent

<div align="center">

**一款基于 Clang/LLVM AST 的智能 C++ 静态分析工具**

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/cpp-code-review)
[![C++17](https://img.shields.io/badge/C++-17-00599C.svg)](https://isocpp.org/)
[![Clang](https://img.shields.io/badge/Powered%20by-Clang%2FLLVM-262D3A.svg)](https://clang.llvm.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

</div>

---

## 📋 目录

- [项目概述](#项目概述)
- [核心特性](#核心特性)
- [技术架构](#技术架构)
- [安装与构建](#安装与构建)
- [使用方法](#使用方法)
- [检测规则](#检测规则)
- [示例输出](#示例输出)
- [开发路线图](#开发路线图)
- [贡献指南](#贡献指南)

---

## 🎯 项目概述

C++ 智能代码审查 Agent 是一款专业的静态分析工具,旨在帮助 C++ 开发团队:

- ✅ **提升代码质量** - 自动发现潜在的 Bug 和安全漏洞
- ✅ **提高开发效率** - 减少人工代码审查的时间和精力
- ✅ **增强代码安全** - 检测常见的安全问题和不安全的编程实践
- ✅ **优化性能** - 提供性能优化建议和最佳实践

### 目标用户

- **C++ 开发者** - 在提交代码前快速发现问题
- **技术负责人** - 保障团队代码质量和统一编码规范
- **QA/安全团队** - 自动化扫描代码库,发现安全隐患

---

## ⭐ 核心特性

### V1.0 (当前版本 - MVP)

#### 🐛 Bug 检测
- **空指针解引用检测** - 发现对 `nullptr`、`NULL` 或 `0` 指针的解引用操作
- **未初始化变量检测** - 识别未初始化的基本类型变量和指针
- **赋值/比较混淆检测** - 捕获条件语句中的赋值操作 `if (a = b)`

#### 🔒 安全分析
- **不安全 C 函数检测** - 标记 `strcpy`、`sprintf`、`gets` 等危险函数
- **缓冲区溢出风险** - 识别可能导致缓冲区溢出的代码模式

#### 📊 智能报告
- **严重性分级** - Critical、High、Medium、Low、Suggestion
- **精准定位** - 准确的文件名、行号、列号
- **修复建议** - 每个问题都附带详细的修复建议
- **彩色控制台输出** - 清晰易读的报告格式

---

## 🏗️ 技术架构

### 核心技术栈

```
┌─────────────────────────────────────────┐
│         CLI Interface (main.cpp)        │
├─────────────────────────────────────────┤
│      Rule Engine (规则引擎框架)          │
│  ┌────────────┬────────────┬─────────┐  │
│  │ Null Ptr   │ Uninit Var │ Unsafe  │  │
│  │   Rule     │    Rule    │C Funcs  │  │
│  └────────────┴────────────┴─────────┘  │
├─────────────────────────────────────────┤
│    AST Parser (基于 libclang)           │
│         Clang/LLVM Frontend             │
├─────────────────────────────────────────┤
│          C++ Source Code                │
└─────────────────────────────────────────┘
```

### 核心组件

1. **AST 解析器** (`src/parser/`)
   - 使用 Clang/LLVM libclang 库
   - 生成完整的抽象语法树
   - 支持 C++11/14/17/20 标准

2. **规则引擎** (`src/rules/`)
   - 可扩展的规则框架
   - 基于访问者模式遍历 AST
   - 支持自定义规则添加

3. **报告生成器** (`src/report/`)
   - 格式化问题输出
   - 严重性分级
   - 彩色终端支持

4. **命令行接口** (`src/cli/`)
   - 简洁的用户交互
   - 灵活的配置选项

---

## 🔧 安装与构建

### 系统要求

- **操作系统**: Linux, macOS, Windows (WSL)
- **编译器**: GCC 7+ 或 Clang 6+
- **CMake**: 3.15+
- **LLVM/Clang**: 10.0+

### 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    llvm-14 \
    llvm-14-dev \
    clang-14 \
    libclang-14-dev
```

#### macOS
```bash
brew install llvm cmake
export LLVM_DIR=/usr/local/opt/llvm/lib/cmake/llvm
export Clang_DIR=/usr/local/opt/llvm/lib/cmake/clang
```

#### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    llvm-devel \
    clang-devel
```

### 构建项目

```bash
# 克隆仓库
git clone https://github.com/yourusername/cpp-code-review.git
cd cpp-code-review

# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake ..

# 编译
make -j$(nproc)

# 安装 (可选)
sudo make install
```

构建成功后,可执行文件位于 `build/cpp-agent`

---

## 🚀 使用方法

### 基本用法

```bash
# 分析单个文件
./cpp-agent scan example.cpp

# 分析整个目录
./cpp-agent scan /path/to/project

# 指定 C++ 标准
./cpp-agent scan main.cpp --std=c++20

# 直接分析文件
./cpp-agent main.cpp
```

### 命令行选项

```
OPTIONS:
  scan <path>         扫描 C++ 文件或目录
  --std=<standard>    指定 C++ 标准 (默认: c++17)
                      可选: c++11, c++14, c++17, c++20
  -h, --help          显示帮助信息
  -v, --version       显示版本信息
```

### 快速开始

尝试分析示例代码:

```bash
# 分析包含问题的代码
./cpp-agent scan examples/test_code.cpp

# 分析良好的代码(应该没有警告)
./cpp-agent scan examples/good_code.cpp
```

---

## 📝 检测规则

### 规则列表

| 规则 ID | 规则名称 | 严重性 | 描述 |
|---------|---------|--------|------|
| **NULL-PTR-001** | 空指针解引用 | CRITICAL | 检测对 `nullptr`、`NULL` 或 `0` 的解引用操作 |
| **UNINIT-VAR-001** | 未初始化变量 | HIGH | 检测未初始化的基本类型变量和指针 |
| **ASSIGN-COND-001** | 条件中的赋值 | HIGH | 检测 `if (a = b)` 而非 `if (a == b)` |
| **UNSAFE-C-FUNC-001** | 不安全 C 函数 | CRITICAL | 检测 `strcpy`、`sprintf`、`gets` 等危险函数 |

### 规则详解

#### 1. 空指针解引用 (NULL-PTR-001)

**检测场景**:
```cpp
int* ptr = nullptr;
*ptr = 42;              // ❌ Critical: 空指针解引用

int* arr = 0;
arr[5] = 10;            // ❌ Critical: 空指针数组访问

MyClass* obj = NULL;
obj->method();          // ❌ Critical: 空指针方法调用
```

**建议修复**:
```cpp
// 方法 1: 使用智能指针
auto ptr = std::make_unique<int>(42);

// 方法 2: 检查后使用
if (ptr != nullptr) {
    *ptr = 42;
}
```

#### 2. 未初始化变量 (UNINIT-VAR-001)

**检测场景**:
```cpp
int x;                  // ❌ High: 未初始化变量
int* ptr;               // ❌ High: 未初始化指针

std::cout << x;         // 使用未初始化的值 - 未定义行为
```

**建议修复**:
```cpp
int x = 0;              // ✅ 显式初始化
int y{};                // ✅ 零初始化
int* ptr = nullptr;     // ✅ 指针初始化为 nullptr
```

#### 3. 条件中的赋值 (ASSIGN-COND-001)

**检测场景**:
```cpp
if (x = 5) {            // ❌ High: 赋值而非比较
    // ...
}

while (flag = false) {  // ❌ High: 永远不会执行
    // ...
}
```

**建议修复**:
```cpp
if (x == 5) {           // ✅ 使用比较运算符
    // ...
}

// 如果确实需要赋值,使用额外的括号明确意图
if ((x = getValue())) {  // ✅ 明确的赋值意图
    // ...
}
```

#### 4. 不安全 C 函数 (UNSAFE-C-FUNC-001)

**检测场景**:
```cpp
char dest[10];
strcpy(dest, src);      // ❌ Critical: 无边界检查

sprintf(buf, "%s", str);// ❌ Critical: 可能溢出
gets(input);            // ❌ Critical: 极度危险
```

**建议修复**:
```cpp
// ✅ 使用 C++ 字符串类
std::string dest = src;

// ✅ 使用安全的 C 函数
snprintf(buf, sizeof(buf), "%s", str);

// ✅ 使用 C++ 流
std::getline(std::cin, input);
```

---

## 📊 示例输出

运行 `./cpp-agent scan examples/test_code.cpp` 的输出示例:

```
╔══════════════════════════════════════════════════════════════════════╗
║      C++ Code Review Agent - Starting Analysis                  ║
╚══════════════════════════════════════════════════════════════════════╝

Configuration:
  C++ Standard: c++17
  Files to analyze: 1

Files:
  - examples/test_code.cpp

Registered 4 analysis rules

Analyzing...

╔══════════════════════════════════════════════════════════════════════╗
║         C++ Code Review Report - Analysis Complete              ║
╚══════════════════════════════════════════════════════════════════════╝

Summary:
  Total issues found: 12
  - Critical: 5
  - High: 7
  - Medium: 0
  - Low: 0
  - Suggestions: 0

Detailed Issues:
═══════════════════════════════════════════════════════════════════════

[Issue #1]
Location: examples/test_code.cpp:9:9
Severity: HIGH
Rule ID: UNINIT-VAR-001
Description: Variable 'x' of type 'int' is declared but not initialized...
Suggestion: Initialize the variable at declaration, e.g., 'int x = <value>;'...
───────────────────────────────────────────────────────────────────────

[Issue #2]
Location: examples/test_code.cpp:16:5
Severity: CRITICAL
Rule ID: NULL-PTR-001
Description: Dereferencing a null pointer will cause undefined behavior...
Suggestion: Check for null before dereferencing, or use smart pointers...
───────────────────────────────────────────────────────────────────────

...
```

---

## 🗺️ 开发路线图

### ✅ V1.0 (当前版本) - MVP
- [x] 基于 AST 的核心解析引擎
- [x] 4 个核心检测规则
- [x] 命令行界面
- [x] 控制台报告输出

### 🚧 V1.5 (计划中) - 增强版
- [ ] 性能优化分析
  - [ ] 内存泄漏检测 (new/delete 不匹配)
  - [ ] 智能指针建议
  - [ ] 循环内拷贝检测
- [ ] Git 集成
  - [ ] 增量分析 (`--diff` 模式)
  - [ ] PR 集成
- [ ] HTML 报告生成
- [ ] 配置文件支持 (`.cpp-agent.yml`)

### 🎯 V2.0 (未来) - 智能化
- [ ] AI/LLM 增强建议
- [ ] VS Code 插件
- [ ] CI/CD 集成 (GitHub Actions, GitLab CI)
- [ ] 污点分析 (Taint Analysis)
- [ ] 整数溢出检测

---

## 🛠️ 技术细节

### 为什么选择 AST 而非正则表达式?

本项目**强制要求**使用 AST (抽象语法树) 进行分析,而**禁止**使用正则表达式:

**AST 的优势**:
- ✅ **语义准确** - 理解代码的真实含义,而非表面文本
- ✅ **低误报率** - 能够区分注释、字符串和实际代码
- ✅ **深度分析** - 支持数据流、控制流分析
- ✅ **可扩展性** - 易于添加复杂的分析规则

**正则表达式的局限**:
- ❌ 无法理解代码语义
- ❌ 容易产生误报和漏报
- ❌ 无法处理复杂的代码结构
- ❌ 难以维护和扩展

### 架构设计原则

1. **可扩展性** - 易于添加新的检测规则
2. **模块化** - 清晰的组件分离
3. **高性能** - 利用 Clang 的高效解析
4. **低误报** - 优先保证准确性

---

## 🤝 贡献指南

我们欢迎所有形式的贡献!

### 如何贡献

1. **Fork 本仓库**
2. **创建特性分支** (`git checkout -b feature/AmazingFeature`)
3. **提交更改** (`git commit -m 'Add some AmazingFeature'`)
4. **推送到分支** (`git push origin feature/AmazingFeature`)
5. **提交 Pull Request**

### 添加新规则

创建新规则的步骤:

1. 在 `src/rules/` 目录下创建新的规则文件
2. 继承 `Rule` 基类和 `RuleVisitor`
3. 实现 `check()` 方法
4. 在 `main.cpp` 中注册新规则

示例代码结构:
```cpp
class MyNewRule : public Rule {
public:
    std::string getRuleId() const override { return "MY-RULE-001"; }
    void check(clang::ASTContext* context, Reporter& reporter) override;
};
```

---

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

---

## 📧 联系方式

- **项目主页**: https://github.com/yourusername/cpp-code-review
- **问题反馈**: https://github.com/yourusername/cpp-code-review/issues

---

## 🙏 致谢

- [Clang/LLVM](https://clang.llvm.org/) - 强大的编译器前端
- [CMake](https://cmake.org/) - 跨平台构建系统
- 所有贡献者和用户

---

<div align="center">

**⭐ 如果觉得这个项目有帮助,请给我们一个 Star!⭐**

Made with ❤️ by C++ Developers, for C++ Developers

</div>
