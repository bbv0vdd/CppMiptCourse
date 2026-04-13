/*
g++ -std=c++23 -Wall -Wextra 11.01.cpp -o 11.01.out
./11.01.out
*/

#include <iostream>

void test_func() {
    std::cout << "Test function ACK" << std::endl;
}

class Wrapper {
private:
    void (*func_ptr)();
    
public:
    Wrapper(void (*func)()) : func_ptr(func) {}
    
    typedef void (*func_type)();
    operator func_type() const {
        return func_ptr;
    }
};

Wrapper test() {
    return Wrapper(test_func);
}

int main() {
    Wrapper function = test();
    function();
    
    return 0;
}