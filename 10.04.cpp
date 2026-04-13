/*
g++ -std=c++23 -Wall -Wextra -Wpedantic -o 10.04 10.04.cpp -lboost_system
./10.04
*/

#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>

namespace ublas = boost::numeric::ublas;

using Matrix2x2 = ublas::matrix<unsigned long long>;

Matrix2x2 multiply_matrices(const Matrix2x2& A, const Matrix2x2& B) {
    Matrix2x2 result(2, 2);
    result(0,0) = A(0,0) * B(0,0) + A(0,1) * B(1,0);
    result(0,1) = A(0,0) * B(0,1) + A(0,1) * B(1,1);
    result(1,0) = A(1,0) * B(0,0) + A(1,1) * B(1,0);
    result(1,1) = A(1,0) * B(0,1) + A(1,1) * B(1,1);
    return result;
}

Matrix2x2 matrix_power(unsigned long long n) {
    Matrix2x2 base(2, 2);
    base(0,0) = 1; base(0,1) = 1;
    base(1,0) = 1; base(1,1) = 0;
    
    Matrix2x2 result(2, 2);
    result(0,0) = 1; result(0,1) = 0;
    result(1,0) = 0; result(1,1) = 1;
    
    while (n > 0) {
        if (n & 1) {
            result = multiply_matrices(result, base);
        }
        base = multiply_matrices(base, base);
        n >>= 1;
    }
    
    return result;
}

unsigned long long fibonacci_matrix(unsigned long long n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    Matrix2x2 M = matrix_power(n - 1);
    return M(0,0);
}

int main() {
    for (unsigned long long n = 0; n <= 50; ++n) {
        std::cout << "F(" << n << ") = " << fibonacci_matrix(n) << std::endl;
    }
    
    return 0;
}