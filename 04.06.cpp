/*
g++ -std=c++23 -Wall -Wextra 04.06.cpp -o 04.06.out
./04.06.out
*/

#include <iostream>
#include <format>
#include <array>
#include <cmath>
#include <cassert>

//////////////////////////////////////////////////////////////////////

consteval double compute_e(double epsilon)
{
    double e = 0.0;
    double term = 1.0;
    int factorial = 1;
    
    while (term > epsilon)
    {
        e += term;
        term /= factorial;
        ++factorial;
    }
    
    return e;
}

//////////////////////////////////////////////////////////////////////

consteval double compute_pi(double epsilon)
{
    double pi = 0.0;
    double term = 1.0;
    int n = 0;
    int sign = 1;
    
    while (std::abs(term) > epsilon)
    {
        pi += term;
        n += 1;
        sign = -sign;
        term = static_cast<double>(sign) / (2 * n + 1);
    }
    
    return 4.0 * pi;
}

//////////////////////////////////////////////////////////////////////

int main()
{
    constexpr std::array<double, 5> epsilons = 
    {
        1e-1, 1e-2, 1e-3
    };
 
    constexpr std::array<double, epsilons.size()> e_values = 
    {
        compute_e(epsilons[0]),
        compute_e(epsilons[1]),
        compute_e(epsilons[2])
    };
    
    constexpr std::array<double, epsilons.size()> pi_values = 
    {
        compute_pi(epsilons[0]),
        compute_pi(epsilons[1]),
        compute_pi(epsilons[2])
    };
    
    static_assert(e_values[0] > 2 && e_values[0] < 3);
    static_assert(e_values[1] > 2.7 && e_values[1] < 2.8);
    
    static_assert(pi_values[1] > 3 && pi_values[1] < 4);
    static_assert(pi_values[1] > 3.1 && pi_values[1] < 3.2);
  
    return 0;
}

//////////////////////////////////////////////////////////////////////