/*
g++ -std=c++23 -Wall -Wextra 04.02.cpp -o 04.02.out
./04.02.out
*/


#include <cassert>
#include <type_traits>

template <typename T>
T max_value(T value)
{
    return value;
}

template <typename T, typename... Args>
T max_value(T first, Args... args)
{
    T max_rest = max_value(args...);
    return (first > max_rest) ? first : max_rest;
}

///////////////////////////////////////////////////////////////////////////////////

template <typename T>
T min_value(T value)
{
    return value;
}

template <typename T, typename... Args>
T min_value(T first, Args... args)
{
    T min_rest = min_value(args...);
    return (first < min_rest) ? first : min_rest;
}

///////////////////////////////////////////////////////////////////////////////////

template <typename... Args>
auto sum(Args... args)
{
    return (... + args);
}

///////////////////////////////////////////////////////////////////////////////////

template <typename... Args>
double average(Args... args)
{
    return (... + args) / static_cast<double>(sizeof...(args));
}

///////////////////////////////////////////////////////////////////////////////////


int main()
{
    // max
    assert(max_value(1.0, 2.0, 3.0, 4.0, 5.0) == 5.0);
    assert(max_value(3.14, 2.71, 1.41) == 3.14);
    assert(max_value(-1.0, -2.0, -3.0) == -1.0);
    assert(max_value(42.0) == 42.0);
    
    // min
    assert(min_value(1.0, 2.0, 3.0, 4.0, 5.0) == 1.0);
    assert(min_value(3.14, 2.71, 1.41) == 1.41);
    assert(min_value(-1.0, -2.0, -3.0) == -3.0);
    assert(min_value(42.0) == 42.0);
    
    // sum
    assert(sum(1.0, 2.0, 3.0, 4.0, 5.0) == 15.0);
    assert(sum(3.14, 2.71) == 5.85);
    assert(sum(-1.0, 1.0, -2.0, 2.0) == 0.0);
    assert(sum(42.0) == 42.0);
    
    // avr
    assert(average(1.0, 2.0, 3.0, 4.0, 5.0) == 3.0);
    assert(average(3.0, 4.0, 5.0) == 4.0);
    assert(average(10.0, 20.0, 30.0) == 20.0);
    assert(average(42.0) == 42.0);
    assert(average(-1.0, 0.0, 1.0) == 0.0);
    
    return 0;
}
