// Example of good C++ code following best practices
// This file should produce no warnings from the code review agent

#include <iostream>
#include <string>
#include <memory>
#include <vector>

// Good: Using modern C++ features and proper initialization
class SafeArray {
private:
    std::vector<int> data_;

public:
    // Good: Proper initialization in constructor
    explicit SafeArray(size_t size) : data_(size, 0) {}

    // Good: Bounds checking
    int get(size_t index) const {
        if (index < data_.size()) {
            return data_[index];
        }
        throw std::out_of_range("Index out of bounds");
    }

    void set(size_t index, int value) {
        if (index < data_.size()) {
            data_[index] = value;
        } else {
            throw std::out_of_range("Index out of bounds");
        }
    }
};

// Good: Using smart pointers instead of raw pointers
std::unique_ptr<int> createInt(int value) {
    return std::make_unique<int>(value);
}

// Good: Proper comparison operator
void checkValue(int x) {
    if (x == 5) {  // Good: Using == for comparison
        std::cout << "x equals 5" << std::endl;
    }
}

// Good: Using std::string instead of C-style strings
void stringOperations() {
    std::string dest = "Hello";
    std::string src = " World";

    // Good: Safe string concatenation
    dest += src;

    std::cout << dest << std::endl;
}

// Good: All variables properly initialized
void properInitialization() {
    int x = 0;  // Good: Initialized
    int y{};    // Good: Zero-initialized using braces
    int* ptr = nullptr;  // Good: Initialized to nullptr

    std::cout << "x: " << x << ", y: " << y << std::endl;

    // Good: Check before using pointer
    if (ptr != nullptr) {
        std::cout << *ptr << std::endl;
    }
}

// Good: RAII pattern for resource management
class FileHandler {
private:
    FILE* file_;

public:
    explicit FileHandler(const char* filename) {
        file_ = fopen(filename, "r");
        if (!file_) {
            throw std::runtime_error("Failed to open file");
        }
    }

    ~FileHandler() {
        if (file_) {
            fclose(file_);  // Good: Resource cleaned up in destructor
        }
    }

    // Delete copy constructor and assignment operator
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
};

// Good: Using references instead of pointers when appropriate
void processValue(const int& value) {
    std::cout << "Value: " << value << std::endl;
}

int main() {
    // Good: All code follows best practices
    checkValue(5);
    stringOperations();
    properInitialization();

    // Good: Using smart pointers - automatic cleanup
    auto value = createInt(42);
    std::cout << "Smart pointer value: " << *value << std::endl;

    // Good: Using RAII-based container
    SafeArray arr(10);
    arr.set(0, 100);
    std::cout << "Array[0]: " << arr.get(0) << std::endl;

    return 0;
}
