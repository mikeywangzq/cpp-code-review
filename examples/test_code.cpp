// Example C++ code with various issues to demonstrate the code review agent
// This file intentionally contains bugs and security issues for testing purposes

#include <iostream>
#include <cstring>

// Example 1: Uninitialized variable
void uninitializedVariable() {
    int x;  // BUG: Uninitialized variable
    int* ptr;  // BUG: Uninitialized pointer

    std::cout << "Value: " << x << std::endl;  // Using uninitialized variable
}

// Example 2: Null pointer dereference
void nullPointerDereference() {
    int* ptr = nullptr;
    *ptr = 42;  // BUG: Dereferencing null pointer

    int* ptr2 = 0;
    int value = *ptr2;  // BUG: Dereferencing null pointer
}

// Example 3: Assignment in condition (common typo)
void assignmentInCondition(int x) {
    if (x = 5) {  // BUG: Should be == instead of =
        std::cout << "x is 5" << std::endl;
    }

    int y = 10;
    while (y = 0) {  // BUG: Assignment instead of comparison
        std::cout << "Loop" << std::endl;
    }
}

// Example 4: Unsafe C-style functions
void unsafeCFunctions() {
    char dest[10];
    char src[] = "This is a very long string that will overflow";

    strcpy(dest, src);  // BUG: Unsafe - no bounds checking, buffer overflow

    char buffer[100];
    sprintf(buffer, "Number: %d", 12345);  // BUG: Unsafe - use snprintf instead

    strcat(dest, "more");  // BUG: Unsafe - use strncat or std::string
}

// Example 5: Multiple issues in one function
int* createArray(int size) {
    int* arr;  // BUG: Uninitialized pointer

    if (size = 0) {  // BUG: Assignment in condition
        return nullptr;
    }

    arr = new int[size];

    // Memory leak - no delete[] anywhere
    return arr;
}

// Example 6: More null pointer issues
void usePointer(int* ptr) {
    if (ptr) {
        std::cout << *ptr << std::endl;
    }

    // Later in code...
    int* p = nullptr;
    p->toString();  // BUG: Calling method on null pointer (if this were valid C++)
}

// Example 7: Array access with null pointer
void arrayAccess() {
    int* arr = nullptr;
    arr[0] = 10;  // BUG: Array access on null pointer
}

int main() {
    // This code won't run properly due to all the bugs!
    // This is for testing the static analyzer only

    uninitializedVariable();
    nullPointerDereference();
    assignmentInCondition(3);
    unsafeCFunctions();

    int* array = createArray(10);
    // BUG: Memory leak - array is never deleted

    return 0;
}
