<div align="center">

# 🛡️ C++ 智能代码审查 Agent

### 基于 Clang/LLVM AST 的专业级静态分析工具

<p align="center">
  <img src="https://img.shields.io/badge/版本-2.0.0-brightgreen?style=for-the-badge&logo=semantic-release" alt="Version"/>
  <img src="https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Powered_by-Clang_LLVM-262D3A?style=for-the-badge&logo=llvm" alt="Clang"/>
  <img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge" alt="License"/>
</p>

<p align="center">
  <strong>🚀 让代码审查更智能 · 让安全漏洞无处可藏 · 让代码质量触手可及</strong>
</p>

<p align="center">
  <a href="#-快速开始">快速开始</a> •
  <a href="#-核心特性">核心特性</a> •
  <a href="#-使用方法">使用方法</a> •
  <a href="#-检测规则">检测规则</a> •
  <a href="#-技术架构">技术架构</a>
</p>

---

</div>

## 📋 目录

<table>
<tr>
<td>

- 💡 [项目概述](#-项目概述)
- ⭐ [核心特性](#-核心特性)
- 🚀 [快速开始](#-快速开始)
- 🎮 [使用方法](#-使用方法)

</td>
<td>

- 📝 [检测规则](#-检测规则)
- 🏗️ [技术架构](#️-技术架构)
- 🗺️ [开发路线图](#️-开发路线图)
- 🤝 [贡献指南](#-贡献指南)

</td>
</tr>
</table>

<br>

## 💡 项目概述

<div align="center">
<h3>🎯 用代码说话,让质量可见</h3>
</div>

**C++ 智能代码审查 Agent** 是一款基于 **Clang/LLVM AST** 技术的专业级 C++ 静态分析工具。它采用先进的抽象语法树分析技术,摆脱传统正则表达式的局限,为您的代码提供精准、深入的质量检测。

<table>
<tr>
<td width="50%">

### 🎁 核心价值

- 🔍 **零误报承诺** - AST 级别分析,告别误报烦恼
- ⚡ **闪电般速度** - 高效解析,秒级完成代码审查
- 🎯 **精准定位** - 文件、行号、列号一目了然
- 💊 **智能修复** - 不仅发现问题,更提供解决方案

</td>
<td width="50%">

### 👥 适用场景

- 💻 **个人开发者** - 提交前的最后一道防线
- 👔 **技术团队** - 统一代码规范的利器
- 🔐 **安全审计** - 发现隐藏的安全隐患
- 📚 **教学培训** - 学习 C++ 最佳实践

</td>
</tr>
</table>

<br>

## ⭐ 核心特性

<div align="center">
<h3>🔥 V2.0 智能化 - 十大检测引擎</h3>
<p><em>基于 AST 的深度语义分析,零正则表达式依赖</em></p>
<p><sub>V1.0: Bug检测 + 安全扫描 | V1.5: 性能优化 + HTML报告 | V2.0 NEW: 高级安全分析</sub></p>
</div>

<table>
<tr>
<td width="50%" valign="top">

### 🐛 Bug 智能检测

<details open>
<summary><b>🎯 空指针解引用检测</b> <code>NULL-PTR-001</code></summary>
<br>

```cpp
int* ptr = nullptr;
*ptr = 42;  // ❌ 立即检测
```

捕获所有 `nullptr`、`NULL`、`0` 的解引用操作,避免程序崩溃。

</details>

<details>
<summary><b>⚠️ 未初始化变量检测</b> <code>UNINIT-VAR-001</code></summary>
<br>

```cpp
int x;  // ❌ 危险!
cout << x;  // 未定义行为
```

识别所有未初始化的基本类型和指针,杜绝未定义行为。

</details>

<details>
<summary><b>🔄 赋值/比较混淆检测</b> <code>ASSIGN-COND-001</code></summary>
<br>

```cpp
if (x = 5) {  // ❌ 应该是 ==
    // 逻辑错误
}
```

捕获 `if (a = b)` 这类经典错误,防止逻辑 Bug。

</details>

</td>
<td width="50%" valign="top">

### 🔒 安全漏洞扫描

<details open>
<summary><b>💣 不安全 C 函数检测</b> <code>UNSAFE-C-FUNC-001</code></summary>
<br>

```cpp
strcpy(dest, src);   // ❌ 缓冲区溢出
sprintf(buf, fmt);   // ❌ 无边界检查
gets(input);         // ❌ 极度危险
```

标记 8+ 种危险函数,建议安全替代方案。

</details>

<br>

### 📊 智能报告系统

<table>
<tr>
<td align="center">🚨<br><b>严重性分级</b></td>
<td align="center">📍<br><b>精准定位</b></td>
</tr>
<tr>
<td align="center">🎨<br><b>彩色输出</b></td>
<td align="center">💡<br><b>修复建议</b></td>
</tr>
</table>

支持 **5 级严重性**: Critical · High · Medium · Low · Suggestion

</td>
</tr>
</table>

<br>

## 🚀 快速开始

<div align="center">
<h3>⚡ 三步启动,立即体验</h3>
</div>

### 📦 一键构建

```bash
# 🎯 克隆项目
git clone https://github.com/yourusername/cpp-code-review.git
cd cpp-code-review

# 🔨 自动化构建 (推荐)
./build.sh

# 🎉 开始使用!
./build/cpp-agent scan examples/test_code.cpp
```

<details>
<summary>📋 <b>手动构建步骤</b> (适合自定义配置)</summary>

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

</details>

### 🎮 基本使用

<table>
<tr>
<td>

**扫描单个文件**
```bash
cpp-agent scan main.cpp
```

</td>
<td>

**扫描整个项目**
```bash
cpp-agent scan ./src
```

</td>
<td>

**指定 C++ 标准**
```bash
cpp-agent scan app.cpp --std=c++20
```

</td>
</tr>
</table>

<br>

### 💻 系统要求

<table>
<tr>
<td align="center">🖥️<br><b>操作系统</b><br>Linux · macOS · WSL</td>
<td align="center">🔧<br><b>编译器</b><br>GCC 7+ · Clang 6+</td>
<td align="center">⚙️<br><b>CMake</b><br>3.15+</td>
<td align="center">🦙<br><b>LLVM</b><br>10.0+</td>
</tr>
</table>

<details>
<summary>🐧 <b>Ubuntu/Debian 依赖安装</b></summary>

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake \
    llvm-14 llvm-14-dev \
    clang-14 libclang-14-dev
```

</details>

<details>
<summary>🍎 <b>macOS 依赖安装</b></summary>

```bash
brew install llvm cmake
export LLVM_DIR=/usr/local/opt/llvm/lib/cmake/llvm
export Clang_DIR=/usr/local/opt/llvm/lib/cmake/clang
```

</details>

<details>
<summary>🎩 <b>Fedora/RHEL 依赖安装</b></summary>

```bash
sudo dnf install -y gcc-c++ cmake llvm-devel clang-devel
```

</details>

<br>

## 🎮 使用方法

### 📖 命令参考

<table>
<tr>
<th>命令</th>
<th>说明</th>
<th>示例</th>
</tr>
<tr>
<td><code>scan &lt;path&gt;</code></td>
<td>扫描文件或目录</td>
<td><code>cpp-agent scan main.cpp</code></td>
</tr>
<tr>
<td><code>--std=&lt;标准&gt;</code></td>
<td>指定 C++ 标准版本</td>
<td><code>--std=c++20</code></td>
</tr>
<tr>
<td><code>-h, --help</code></td>
<td>显示帮助信息</td>
<td><code>cpp-agent --help</code></td>
</tr>
<tr>
<td><code>-v, --version</code></td>
<td>显示版本信息</td>
<td><code>cpp-agent --version</code></td>
</tr>
<tr style="background-color: #e7f3ff;">
<td><code>--html</code></td>
<td>生成 HTML 报告 <span style="color: #28a745;">V1.5</span></td>
<td><code>cpp-agent scan main.cpp --html</code></td>
</tr>
<tr style="background-color: #e7f3ff;">
<td><code>--html-output=&lt;文件&gt;</code></td>
<td>指定 HTML 输出文件 <span style="color: #28a745;">V1.5</span></td>
<td><code>--html-output=report.html</code></td>
</tr>
</table>

### 🎯 实战示例

```bash
# 💡 体验问题检测 (会发现 12+ 个问题)
./cpp-agent scan examples/test_code.cpp

# ✅ 查看最佳实践 (应该零警告)
./cpp-agent scan examples/good_code.cpp

# 🆕 V1.5 性能分析示例 (内存泄漏、智能指针、循环优化)
./cpp-agent scan examples/v1.5_features.cpp --html

# 🔥 V2.0 高级安全示例 (整数溢出、Use-After-Free、缓冲区溢出)
./cpp-agent scan examples/v2.0_features.cpp --html

# 🚀 分析真实项目并生成 HTML 报告
./cpp-agent scan /path/to/your/project --std=c++17 --html-output=my_report.html
```

<br>

## 📝 检测规则

<div align="center">
<h3>🔍 十大核心规则 · 全方位代码守护</h3>
<p><sub>V1.0: 4条基础规则 | V1.5: +3条性能优化 | V2.0 NEW: +3条高级安全</sub></p>
</div>

### 📊 规则总览

<table>
<thead>
<tr>
<th width="20%">规则 ID</th>
<th width="25%">规则名称</th>
<th width="15%">严重性</th>
<th width="40%">检测内容</th>
</tr>
</thead>
<tbody>
<tr>
<td><code>NULL-PTR-001</code></td>
<td>🎯 空指针解引用</td>
<td><img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/></td>
<td>捕获所有空指针解引用操作</td>
</tr>
<tr>
<td><code>UNINIT-VAR-001</code></td>
<td>⚠️ 未初始化变量</td>
<td><img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/></td>
<td>识别未初始化的变量和指针</td>
</tr>
<tr>
<td><code>ASSIGN-COND-001</code></td>
<td>🔄 赋值比较混淆</td>
<td><img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/></td>
<td>检测条件语句中的赋值错误</td>
</tr>
<tr>
<td><code>UNSAFE-C-FUNC-001</code></td>
<td>💣 不安全 C 函数</td>
<td><img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/></td>
<td>标记危险的 C 风格函数调用</td>
</tr>
<tr style="background-color: #e7f3ff;">
<td><code>MEMORY-LEAK-001</code></td>
<td>🧠 内存泄漏检测 <span style="color: #28a745;">V1.5</span></td>
<td><img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/></td>
<td>检测 new/delete 不匹配的内存泄漏</td>
</tr>
<tr style="background-color: #e7f3ff;">
<td><code>SMART-PTR-001</code></td>
<td>💡 智能指针建议 <span style="color: #28a745;">V1.5</span></td>
<td><img src="https://img.shields.io/badge/-SUGGESTION-green?style=flat-square"/></td>
<td>建议使用智能指针替代裸指针</td>
</tr>
<tr style="background-color: #e7f3ff;">
<td><code>LOOP-COPY-001</code></td>
<td>🔄 循环拷贝优化 <span style="color: #28a745;">V1.5</span></td>
<td><img src="https://img.shields.io/badge/-MEDIUM-yellow?style=flat-square"/></td>
<td>识别循环中的昂贵拷贝操作</td>
</tr>
<tr style="background-color: #fff3cd;">
<td><code>INTEGER-OVERFLOW-001</code></td>
<td>🔢 整数溢出检测 <span style="color: #0066cc;">V2.0</span></td>
<td><img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/></td>
<td>检测算术运算和类型转换中的溢出</td>
</tr>
<tr style="background-color: #fff3cd;">
<td><code>USE-AFTER-FREE-001</code></td>
<td>🕵️ Use-After-Free 检测 <span style="color: #0066cc;">V2.0</span></td>
<td><img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/></td>
<td>检测 delete 后继续使用指针</td>
</tr>
<tr style="background-color: #fff3cd;">
<td><code>BUFFER-OVERFLOW-001</code></td>
<td>💥 缓冲区溢出检测 <span style="color: #0066cc;">V2.0</span></td>
<td><img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/></td>
<td>检测数组越界访问</td>
</tr>
</tbody>
</table>

<br>

### 🎯 规则详解

<table>
<tr>
<td width="50%">

#### 🎯 规则 1: 空指针解引用

<kbd>NULL-PTR-001</kbd> <img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/>

**❌ 危险代码**
```cpp
int* ptr = nullptr;
*ptr = 42;  // 程序崩溃!

int* arr = 0;
arr[5] = 10;  // 未定义行为

MyClass* obj = NULL;
obj->method();  // 段错误
```

**✅ 安全方案**
```cpp
// 方案 1: 智能指针
auto ptr = std::make_unique<int>(42);

// 方案 2: 空指针检查
if (ptr != nullptr) {
    *ptr = 42;
}
```

</td>
<td width="50%">

#### ⚠️ 规则 2: 未初始化变量

<kbd>UNINIT-VAR-001</kbd> <img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/>

**❌ 危险代码**
```cpp
int x;       // 未初始化
int* ptr;    // 野指针

std::cout << x;    // 未定义行为!
*ptr = 100;        // 灾难性后果
```

**✅ 安全方案**
```cpp
int x = 0;          // 显式初始化
int y{};            // 零初始化
int* ptr = nullptr; // 初始化为空

// 或使用 auto
auto z = 42;
```

</td>
</tr>
<tr>
<td width="50%">

#### 🔄 规则 3: 赋值比较混淆

<kbd>ASSIGN-COND-001</kbd> <img src="https://img.shields.io/badge/-HIGH-orange?style=flat-square"/>

**❌ 危险代码**
```cpp
if (x = 5) {         // 赋值!
    // 总是执行
}

while (flag = 0) {   // 永不执行
    // 死循环?
}
```

**✅ 安全方案**
```cpp
if (x == 5) {        // 正确比较
    // ...
}

// 如需赋值,明确意图
if ((x = getValue()) != 0) {
    // OK
}
```

</td>
<td width="50%">

#### 💣 规则 4: 不安全 C 函数

<kbd>UNSAFE-C-FUNC-001</kbd> <img src="https://img.shields.io/badge/-CRITICAL-red?style=flat-square"/>

**❌ 危险代码**
```cpp
char buf[10];
strcpy(buf, src);    // 缓冲区溢出!
sprintf(buf, fmt);   // 无边界检查
gets(input);         // 已废弃,极危险
```

**✅ 安全方案**
```cpp
// 使用 C++ 字符串
std::string buf = src;

// 使用安全函数
snprintf(buf, sizeof(buf), fmt);

// 使用 C++ 流
std::getline(std::cin, input);
```

</td>
</tr>
</table>

<br>

## 📊 示例输出

<div align="center">
<h3>🎨 精美的控制台报告</h3>
<p><em>彩色输出 · 问题分级 · 修复建议</em></p>
</div>

运行命令 `./cpp-agent scan examples/test_code.cpp` 后,您将看到:

```
╔══════════════════════════════════════════════════════════════════════╗
║      🛡️ C++ Code Review Agent - 开始分析                        ║
╚══════════════════════════════════════════════════════════════════════╝

配置信息:
  📌 C++ 标准: c++17
  📁 分析文件: 1 个

待分析文件:
  - examples/test_code.cpp

🔍 已注册 10 条分析规则 (V2.0)
⚡ 正在分析...

╔══════════════════════════════════════════════════════════════════════╗
║         ✅ C++ 代码审查报告 - 分析完成                          ║
╚══════════════════════════════════════════════════════════════════════╝

📈 问题统计:
  总问题数: 12
  - 🚨 Critical (严重): 5
  - ⚠️  High (高):     7
  - 🟡 Medium (中):    0
  - 🔵 Low (低):       0
  - 💡 Suggestion:    0

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

[问题 #1] ⚠️  HIGH
📍 位置: examples/test_code.cpp:9:9
🏷️  规则: UNINIT-VAR-001
📝 描述: 变量 'x' 类型为 'int',声明但未初始化...
💡 建议: 在声明时初始化变量,如 'int x = 0;' 或使用 '{}' 零初始化

─────────────────────────────────────────────────────────────────────

[问题 #2] 🚨 CRITICAL
📍 位置: examples/test_code.cpp:16:5
🏷️  规则: NULL-PTR-001
📝 描述: 解引用空指针将导致未定义行为和程序崩溃...
💡 建议: 使用前检查指针是否为空,或使用智能指针 (std::unique_ptr)

─────────────────────────────────────────────────────────────────────

... (更多问题)

✨ 分析完成! 请查看并修复上述问题。
```

> 💡 **提示**: 使用 `examples/good_code.cpp` 查看零问题的理想输出

<br>

## 🏗️ 技术架构

<div align="center">
<h3>🔧 模块化设计 · 可扩展架构</h3>
</div>

```
                    ┌─────────────────────────────────┐
                    │   💻 命令行界面 (CLI)           │
                    │   • 参数解析                    │
                    │   • 用户交互                    │
                    └──────────────┬──────────────────┘
                                   │
                    ┌──────────────▼──────────────────┐
                    │   🎮 规则引擎 (Rule Engine)     │
                    │   ┌──────┬──────┬──────┬──────┐ │
                    │   │ Null │Uninit│Assign│Unsafe│ │
                    │   │ Ptr  │ Var  │ Cond │C Func│ │
                    │   └──────┴──────┴──────┴──────┘ │
                    └──────────────┬──────────────────┘
                                   │
                    ┌──────────────▼──────────────────┐
                    │   🌲 AST 解析器 (Parser)        │
                    │   • Clang/LLVM LibTooling       │
                    │   • 深度语法分析                │
                    │   • 支持 C++11/14/17/20         │
                    └──────────────┬──────────────────┘
                                   │
                    ┌──────────────▼──────────────────┐
                    │   📝 报告生成器 (Reporter)      │
                    │   • 问题收集                    │
                    │   • 严重性评估                  │
                    │   • 彩色输出                    │
                    └─────────────────────────────────┘
```

<table>
<tr>
<td width="25%" align="center">

### 🎯 AST 解析
**100% 语义准确**

零正则表达式<br>
深度代码理解

</td>
<td width="25%" align="center">

### 🔌 可扩展性
**插件化规则**

轻松添加规则<br>
模块化设计

</td>
<td width="25%" align="center">

### ⚡ 高性能
**Clang 级速度**

秒级分析<br>
并行处理

</td>
<td width="25%" align="center">

### 🎨 智能报告
**精准定位**

问题分级<br>
修复建议

</td>
</tr>
</table>

<br>

## 🗺️ 开发路线图

<div align="center">
<h3>🚀 从 MVP 到智能化的进化之路</h3>
</div>

<table>
<tr>
<td width="33%" valign="top">

### ✅ V1.0 MVP
<sub>当前版本</sub>

- [x] 🌲 AST 核心引擎
- [x] 🔍 4 大检测规则
- [x] 💻 命令行工具
- [x] 📊 彩色报告
- [x] 📚 完整文档
- [x] 🧪 测试用例

<br>

<div align="center">
<img src="https://img.shields.io/badge/状态-已发布-brightgreen?style=for-the-badge"/>
</div>

</td>
<td width="33%" valign="top">

### ✅ V1.5 增强版
<sub>当前版本</sub>

**性能分析**
- [x] 🧠 内存泄漏检测
- [x] 💡 智能指针建议
- [x] 🔄 循环优化检测

**Git 集成**
- [ ] 📊 增量分析
- [ ] 🔀 PR 自动审查

**功能增强**
- [x] 📄 HTML 报告
- [x] ⚙️ 配置文件支持

<br>

<div align="center">
<img src="https://img.shields.io/badge/状态-已发布-brightgreen?style=for-the-badge"/>
</div>

</td>
<td width="33%" valign="top">

### ✅ V2.0 智能化
<sub>当前版本</sub>

**高级安全分析**
- [x] 🔢 整数溢出检测
- [x] 🕵️ Use-After-Free 检测
- [x] 💥 缓冲区溢出检测

**未来计划**
- [ ] 🤖 LLM 智能建议
- [ ] 🔮 代码修复方案
- [ ] 🎨 VS Code 插件
- [ ] 🔄 CI/CD 集成
- [ ] 🐙 GitHub Actions
- [ ] 🔬 数据流污点分析

<br>

<div align="center">
<img src="https://img.shields.io/badge/状态-已发布-brightgreen?style=for-the-badge"/>
</div>

</td>
</tr>
</table>

<br>

## 🛠️ 技术细节

<div align="center">
<h3>💎 为什么选择 AST?</h3>
<p><em>深度语义分析 vs 表面文本匹配</em></p>
</div>

<table>
<tr>
<td width="50%" align="center">

### ✅ AST 分析 (本项目)

<table>
<tr><td>🎯 <b>语义准确</b></td><td>理解代码真实含义</td></tr>
<tr><td>🔍 <b>零误报</b></td><td>区分注释和代码</td></tr>
<tr><td>🚀 <b>深度分析</b></td><td>数据流/控制流</td></tr>
<tr><td>🔌 <b>可扩展</b></td><td>复杂规则支持</td></tr>
<tr><td>⚡ <b>高效</b></td><td>Clang 级性能</td></tr>
</table>

<br>

<img src="https://img.shields.io/badge/推荐指数-⭐⭐⭐⭐⭐-gold?style=for-the-badge"/>

</td>
<td width="50%" align="center">

### ❌ 正则表达式

<table>
<tr><td>🚫 <b>无语义理解</b></td><td>仅匹配文本</td></tr>
<tr><td>⚠️ <b>高误报</b></td><td>误判注释/字符串</td></tr>
<tr><td>📉 <b>浅层分析</b></td><td>无法深入</td></tr>
<tr><td>🔒 <b>难扩展</b></td><td>复杂规则难写</td></tr>
<tr><td>🐌 <b>效率低</b></td><td>大项目慢</td></tr>
</table>

<br>

<img src="https://img.shields.io/badge/本项目-禁止使用-red?style=for-the-badge"/>

</td>
</tr>
</table>

### 🏛️ 架构设计原则

<table>
<tr>
<td align="center">🧩<br><b>模块化</b><br>清晰组件分离</td>
<td align="center">🔌<br><b>可扩展</b><br>轻松添加规则</td>
<td align="center">⚡<br><b>高性能</b><br>Clang 级速度</td>
<td align="center">🎯<br><b>低误报</b><br>准确性优先</td>
</tr>
</table>

<br>

## 🤝 贡献指南

<div align="center">
<h3>👥 欢迎加入我们!</h3>
<p><em>每一个贡献都让 C++ 社区更美好</em></p>
</div>

### 🚀 参与方式

<table>
<tr>
<td width="25%" align="center">

**1️⃣ Fork**

Fork 本仓库到<br>你的账号下

</td>
<td width="25%" align="center">

**2️⃣ 分支**

创建特性分支<br>
`git checkout -b feature/xxx`

</td>
<td width="25%" align="center">

**3️⃣ 提交**

提交你的改动<br>
`git commit -m "feat: xxx"`

</td>
<td width="25%" align="center">

**4️⃣ PR**

推送并创建<br>
Pull Request

</td>
</tr>
</table>

### 🎨 添加新规则

<details>
<summary><b>📖 点击查看详细步骤</b></summary>

<br>

**第 1 步**: 创建规则文件

在 `src/rules/` 目录下创建 `my_new_rule.h` 和 `my_new_rule.cpp`

**第 2 步**: 继承基类

```cpp
class MyNewRule : public Rule {
public:
    std::string getRuleId() const override {
        return "MY-RULE-001";
    }

    std::string getRuleName() const override {
        return "My New Rule";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override {
        MyNewRuleVisitor visitor(context, reporter);
        visitor.TraverseDecl(context->getTranslationUnitDecl());
    }
};
```

**第 3 步**: 注册规则

在 `src/main.cpp` 中添加:

```cpp
engine.registerRule(std::make_unique<MyNewRule>());
```

**第 4 步**: 测试并提交 PR

</details>

<br>

## 📄 许可证

<div align="center">

<img src="https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge&logo=opensourceinitiative&logoColor=white" alt="MIT License"/>

本项目采用 **MIT 许可证** 开源

[查看完整许可证](LICENSE)

</div>

<br>

## 📧 联系我们

<div align="center">

<table>
<tr>
<td align="center">
<a href="https://github.com/yourusername/cpp-code-review">
<img src="https://img.shields.io/badge/GitHub-项目主页-181717?style=for-the-badge&logo=github" alt="GitHub"/>
</a>
</td>
<td align="center">
<a href="https://github.com/yourusername/cpp-code-review/issues">
<img src="https://img.shields.io/badge/Issues-问题反馈-green?style=for-the-badge&logo=github" alt="Issues"/>
</a>
</td>
<td align="center">
<a href="https://github.com/yourusername/cpp-code-review/discussions">
<img src="https://img.shields.io/badge/Discussions-交流讨论-blue?style=for-the-badge&logo=github" alt="Discussions"/>
</a>
</td>
</tr>
</table>

</div>

<br>

## 🙏 致谢

<div align="center">

感谢以下优秀的开源项目和社区

<table>
<tr>
<td align="center" width="33%">
<a href="https://clang.llvm.org/">
<img src="https://img.shields.io/badge/Clang%2FLLVM-强大的编译器前端-262D3A?style=for-the-badge&logo=llvm" alt="Clang"/>
</a>
<br>
<sub>提供核心 AST 解析能力</sub>
</td>
<td align="center" width="33%">
<a href="https://cmake.org/">
<img src="https://img.shields.io/badge/CMake-跨平台构建系统-064F8C?style=for-the-badge&logo=cmake" alt="CMake"/>
</a>
<br>
<sub>简化项目构建流程</sub>
</td>
<td align="center" width="33%">
<img src="https://img.shields.io/badge/C++_Community-活跃的开发者社区-00599C?style=for-the-badge&logo=cplusplus" alt="C++ Community"/>
<br>
<sub>灵感与支持来源</sub>
</td>
</tr>
</table>

**特别感谢所有贡献者和 Star 支持者!** 🌟

</div>

---

<br>

<div align="center">

## 🌟 如果这个项目对你有帮助

### 请给我们一个 Star ⭐

<br>

<table>
<tr>
<td align="center" width="50%">

### 📊 项目统计

<img src="https://img.shields.io/github/stars/yourusername/cpp-code-review?style=social" alt="Stars"/>
<img src="https://img.shields.io/github/forks/yourusername/cpp-code-review?style=social" alt="Forks"/>
<img src="https://img.shields.io/github/watchers/yourusername/cpp-code-review?style=social" alt="Watchers"/>

</td>
<td align="center" width="50%">

### 🔥 活跃度

<img src="https://img.shields.io/github/last-commit/yourusername/cpp-code-review?style=flat-square" alt="Last Commit"/>
<img src="https://img.shields.io/github/commit-activity/m/yourusername/cpp-code-review?style=flat-square" alt="Commit Activity"/>

</td>
</tr>
</table>

<br>

---

<br>

<h3>
🎯 由 C++ 开发者打造 · 为 C++ 开发者服务
</h3>

<p>
<sub>
让代码审查更智能 | 让安全检测更准确 | 让 C++ 开发更高效
</sub>
</p>

<br>

**Made with ❤️ and ☕ by the C++ Community**

<br>

<img src="https://img.shields.io/badge/Powered_by-AST_Technology-blueviolet?style=for-the-badge"/>
<img src="https://img.shields.io/badge/No-Regex-red?style=for-the-badge"/>
<img src="https://img.shields.io/badge/100%25-Open_Source-brightgreen?style=for-the-badge"/>

</div>
