// V2.0 新特性演示代码
// 本文件展示 C++ Code Review Agent V2.0 的三个高级安全检测规则

#include <iostream>
#include <cstdint>

// ===== 1. 整数溢出检测 (INTEGER-OVERFLOW-001) =====

void integerOverflowExample1() {
    // 问题: 8位整数加法可能溢出
    uint8_t a = 200;
    uint8_t b = 100;
    uint8_t result = a + b;  // 溢出! 200 + 100 = 300 > 255

    std::cout << "Result: " << static_cast<int>(result) << std::endl;
}

void integerOverflowExample2() {
    // 问题: 16位整数乘法可能溢出
    int16_t x = 300;
    int16_t y = 200;
    int16_t product = x * y;  // 溢出! 300 * 200 = 60000 > 32767

    std::cout << "Product: " << product << std::endl;
}

void integerOverflowExample3() {
    // 问题: 32位整数乘法可能溢出
    int32_t num1 = 100000;
    int32_t num2 = 50000;
    int32_t result = num1 * num2;  // 溢出! 5,000,000,000 > 2^31-1

    std::cout << "Result: " << result << std::endl;
}

void narrowingConversionExample1() {
    // 问题: 64位转32位,可能截断数据
    int64_t large_value = 5000000000LL;  // 5 billion
    int32_t small_value = (int32_t)large_value;  // 截断!

    std::cout << "Truncated: " << small_value << std::endl;
}

void narrowingConversionExample2() {
    // 问题: long 转 short 可能截断
    long big_num = 100000;
    short small_num = static_cast<short>(big_num);  // 可能截断

    std::cout << "Converted: " << small_num << std::endl;
}

// ===== 2. Use-After-Free 检测 (USE-AFTER-FREE-001) =====

void useAfterFreeExample1() {
    // 问题: delete 后使用指针
    int* ptr = new int(42);

    delete ptr;

    // Use-after-free!
    std::cout << *ptr << std::endl;  // 危险!
}

void useAfterFreeExample2() {
    // 问题: delete 后通过成员访问
    struct Data {
        int value;
        void print() { std::cout << value << std::endl; }
    };

    Data* obj = new Data{100};

    delete obj;

    obj->print();  // Use-after-free!
}

void useAfterFreeExample3() {
    // 问题: delete[] 后访问数组
    int* arr = new int[10];

    for (int i = 0; i < 10; ++i) {
        arr[i] = i;
    }

    delete[] arr;

    // Use-after-free!
    std::cout << arr[5] << std::endl;
}

void useAfterFreeExample4() {
    // 问题: 条件删除后使用
    int* data = new int(123);
    bool should_delete = true;

    if (should_delete) {
        delete data;
    }

    // Use-after-free (可能)
    processData(data);  // 如果 should_delete 为 true,这里就有问题
}

class Resource {
public:
    Resource() { std::cout << "Resource created\n"; }
    ~Resource() { std::cout << "Resource destroyed\n"; }
    void use() { std::cout << "Using resource\n"; }
};

void useAfterFreeExample5() {
    // 问题: 对象删除后调用方法
    Resource* res = new Resource();

    delete res;

    res->use();  // Use-after-free!
}

// ===== 3. 缓冲区溢出检测 (BUFFER-OVERFLOW-001) =====

void bufferOverflowExample1() {
    // 问题: 常量索引超出数组边界
    int arr[5] = {1, 2, 3, 4, 5};

    int value = arr[10];  // 越界! 数组只有 5 个元素 (索引 0-4)

    std::cout << value << std::endl;
}

void bufferOverflowExample2() {
    // 问题: 负数索引
    int buffer[10];

    buffer[-1] = 100;  // 负索引,缓冲区下溢!
}

void bufferOverflowExample3() {
    // 问题: 循环越界写入
    char str[10];

    for (int i = 0; i <= 10; ++i) {  // 注意: <= 10 会导致越界
        str[i] = 'A';  // 当 i=10 时越界
    }
}

void bufferOverflowExample4() {
    // 问题: 多维数组越界
    int matrix[3][3];

    matrix[3][0] = 99;  // 第一维越界! 有效范围是 0-2
}

void bufferOverflowExample5() {
    // 问题: 小数组的非常量索引访问
    int small_array[5];
    int index = getUserInput();  // 用户输入

    small_array[index] = 100;  // 建议添加边界检查
}

void bufferOverflowExample6() {
    // 问题: 明显的越界访问
    uint8_t bytes[8];

    bytes[0] = 0x01;
    bytes[7] = 0xFF;
    bytes[8] = 0xAA;  // 越界! 有效索引: 0-7
}

// ===== 正确的写法示例 =====

namespace correct_examples {

// 1. 整数溢出安全写法
void safeIntegerArithmetic() {
    // 使用更大的类型
    uint8_t a = 200;
    uint8_t b = 100;
    uint32_t result = static_cast<uint32_t>(a) + static_cast<uint32_t>(b);

    std::cout << "Safe result: " << result << std::endl;
}

void safeNarrowingConversion() {
    // 添加范围检查
    int64_t large_value = 5000000000LL;

    if (large_value >= INT32_MIN && large_value <= INT32_MAX) {
        int32_t small_value = static_cast<int32_t>(large_value);
        std::cout << "Safe conversion: " << small_value << std::endl;
    } else {
        std::cerr << "Error: Value out of range for int32_t\n";
    }
}

// 2. Use-After-Free 安全写法
void safePointerUsage1() {
    // 使用智能指针
    auto ptr = std::make_unique<int>(42);

    std::cout << *ptr << std::endl;

    // ptr 自动释放,无法在释放后使用
}

void safePointerUsage2() {
    // delete 后设置为 nullptr
    int* ptr = new int(42);

    std::cout << *ptr << std::endl;

    delete ptr;
    ptr = nullptr;  // 防止悬空指针

    if (ptr != nullptr) {
        std::cout << *ptr << std::endl;  // 不会执行
    }
}

// 3. 缓冲区溢出安全写法
void safeArrayAccess1() {
    // 使用 std::array 和边界检查
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    try {
        int value = arr.at(2);  // 安全访问,会进行边界检查
        std::cout << value << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Array index out of range: " << e.what() << std::endl;
    }
}

void safeArrayAccess2() {
    // 使用 std::vector 和边界检查
    std::vector<int> vec = {1, 2, 3, 4, 5};

    size_t index = getUserInput();

    if (index < vec.size()) {
        vec[index] = 100;  // 安全
    } else {
        std::cerr << "Index out of bounds\n";
    }
}

void safeArrayAccess3() {
    // 手动边界检查
    int arr[5] = {1, 2, 3, 4, 5};
    int index = getUserInput();

    const int ARRAY_SIZE = 5;
    if (index >= 0 && index < ARRAY_SIZE) {
        arr[index] = 100;  // 安全
    } else {
        std::cerr << "Invalid index\n";
    }
}

} // namespace correct_examples

// 辅助函数
int getUserInput() {
    // 模拟用户输入
    return 3;
}

void processData(int* data) {
    if (data != nullptr) {
        std::cout << "Processing: " << *data << std::endl;
    }
}

int main() {
    std::cout << "=== V2.0 高级安全特性演示 ===" << std::endl;
    std::cout << "运行 cpp-agent 分析此文件以查看:" << std::endl;
    std::cout << "1. 整数溢出检测 (INTEGER-OVERFLOW-001)" << std::endl;
    std::cout << "2. Use-After-Free 检测 (USE-AFTER-FREE-001)" << std::endl;
    std::cout << "3. 缓冲区溢出检测 (BUFFER-OVERFLOW-001)" << std::endl;
    std::cout << std::endl;
    std::cout << "示例命令:" << std::endl;
    std::cout << "  cpp-agent scan examples/v2.0_features.cpp --html" << std::endl;
    std::cout << "  cpp-agent scan examples/v2.0_features.cpp --html-output=v2_report.html" << std::endl;

    return 0;
}
