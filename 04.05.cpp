/*
g++ -std=c++23 -Wall -Wextra 04.05.cpp -o 04.05.out
./04.05.out
*/

#include <limits>

template <int N>
struct Fibonacci
{
    static_assert(N >= 0, "index must be non-negative");
    
    static_assert(Fibonacci<N - 2>::value <= std::numeric_limits<int>::max() - Fibonacci<N - 1>::value, "overflow");

    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};


template <>
struct Fibonacci<0>
{
    static constexpr int value = 0;
};

template <>
struct Fibonacci<1>
{
    static constexpr int value = 1;
};


template <int N>
inline constexpr int fibonacci_v = Fibonacci<N>::value;


int main()
{
    static_assert(fibonacci_v<0> == 0);
    static_assert(fibonacci_v<1> == 1);
    static_assert(fibonacci_v<2> == 1);
    static_assert(fibonacci_v<3> == 2);
    static_assert(fibonacci_v<4> == 3);
    static_assert(fibonacci_v<5> == 5);
    static_assert(fibonacci_v<6> == 8);
    
    //[[maybe_unused]] auto x = fibonacci_v<50>; // overflow

    return 0;
}

////////////////////////////////////////////////////////////////////////