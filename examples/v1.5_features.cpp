// V1.5 新特性演示代码
// 本文件展示 C++ Code Review Agent V1.5 的三个新检测规则

#include <iostream>
#include <vector>
#include <string>
#include <memory>

// ===== 1. 内存泄漏检测 (MEMORY-LEAK-001) =====

void memoryLeakExample1() {
    // 问题: new 分配内存但没有 delete
    int* data = new int[100];

    // 使用数据
    for (int i = 0; i < 100; ++i) {
        data[i] = i;
    }

    // 忘记 delete[] data; - 内存泄漏!
}

class Resource {
public:
    Resource() { std::cout << "Resource created\n"; }
    ~Resource() { std::cout << "Resource destroyed\n"; }
};

void memoryLeakExample2() {
    // 问题: new 创建对象但没有 delete
    Resource* res = new Resource();

    res->~Resource();  // 手动调用析构函数是不够的
    // 忘记 delete res; - 内存泄漏!
}

void memoryLeakExample3() {
    // 问题: 条件分支导致可能的内存泄漏
    char* buffer = new char[1024];

    int result = processData();

    if (result < 0) {
        std::cerr << "Error occurred\n";
        return;  // 提前返回,忘记 delete[] buffer
    }

    delete[] buffer;  // 只有正常路径会释放
}

// ===== 2. 智能指针建议 (SMART-PTR-001) =====

void rawPointerExample1() {
    // 建议: 应该使用 std::unique_ptr
    int* value = new int(42);

    std::cout << *value << std::endl;

    delete value;  // 手动管理容易出错
}

class DataProcessor {
private:
    int* buffer_;  // 建议: 使用 std::unique_ptr<int[]>

public:
    DataProcessor(int size) {
        buffer_ = new int[size];  // 建议: 使用智能指针
    }

    ~DataProcessor() {
        delete[] buffer_;  // 手动管理生命周期
    }
};

void rawPointerExample2() {
    // 建议: 使用 std::make_unique
    std::string* message = new std::string("Hello, World!");

    processMessage(*message);

    delete message;  // 如果 processMessage 抛出异常,这里不会执行
}

// ===== 3. 循环内拷贝优化 (LOOP-COPY-001) =====

void loopCopyExample1() {
    std::vector<std::string> names = {
        "Alice", "Bob", "Charlie", "David", "Eve"
    };

    // 问题: 循环中按值传递,导致不必要的拷贝
    for (std::string name : names) {  // 应该使用 const std::string&
        std::cout << name << std::endl;
    }
}

void loopCopyExample2() {
    std::vector<std::vector<int>> matrix = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15}
    };

    // 问题: 嵌套容器的拷贝开销很大
    for (std::vector<int> row : matrix) {  // 应该使用 const std::vector<int>&
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

struct LargeObject {
    int data[1000];
    std::string description;
    std::vector<double> values;
};

void loopCopyExample3() {
    std::vector<LargeObject> objects(100);

    // 问题: 大对象的拷贝非常昂贵
    for (LargeObject obj : objects) {  // 应该使用 const LargeObject&
        processObject(obj);
    }
}

void loopCopyExample4() {
    std::vector<std::string> paths;
    for (int i = 0; i < 1000; ++i) {
        paths.push_back("/path/to/file" + std::to_string(i));
    }

    // 问题: 传统 for 循环中也可能有拷贝问题
    for (size_t i = 0; i < paths.size(); ++i) {
        std::string path = paths[i];  // 不必要的拷贝,应该使用 const std::string&
        validatePath(path);
    }
}

// ===== 正确的写法示例 =====

namespace correct_examples {

// 1. 使用智能指针避免内存泄漏
void smartPointerExample() {
    // 使用 unique_ptr 自动管理内存
    auto data = std::make_unique<int[]>(100);

    for (int i = 0; i < 100; ++i) {
        data[i] = i;
    }

    // 不需要手动 delete,自动释放
}

void smartPointerExample2() {
    // 使用 unique_ptr 管理对象
    auto res = std::make_unique<Resource>();

    // 异常安全,自动清理
}

// 2. 使用引用避免循环中的拷贝
void optimizedLoop1() {
    std::vector<std::string> names = {
        "Alice", "Bob", "Charlie", "David", "Eve"
    };

    // 使用 const 引用,避免拷贝
    for (const std::string& name : names) {
        std::cout << name << std::endl;
    }
}

void optimizedLoop2() {
    std::vector<std::vector<int>> matrix = {
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15}
    };

    // 使用 const 引用
    for (const std::vector<int>& row : matrix) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

void optimizedLoop3() {
    std::vector<LargeObject> objects(100);

    // 使用 const 引用避免大对象拷贝
    for (const LargeObject& obj : objects) {
        processObject(obj);
    }
}

} // namespace correct_examples

int main() {
    std::cout << "=== V1.5 特性演示 ===" << std::endl;
    std::cout << "运行 cpp-agent 分析此文件以查看:" << std::endl;
    std::cout << "1. 内存泄漏检测 (MEMORY-LEAK-001)" << std::endl;
    std::cout << "2. 智能指针建议 (SMART-PTR-001)" << std::endl;
    std::cout << "3. 循环拷贝优化 (LOOP-COPY-001)" << std::endl;
    std::cout << std::endl;
    std::cout << "示例命令:" << std::endl;
    std::cout << "  cpp-agent scan examples/v1.5_features.cpp --html" << std::endl;

    return 0;
}
