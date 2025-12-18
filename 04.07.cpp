/*
g++ -std=c++23 -Wall -Wextra 04.07.cpp -o 04.07.out
./04.07.out
*/

//////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <numeric>

//////////////////////////////////////////////////////////////////////////////////////////

template < int N = 0, int D = 1 > struct Ratio
{
	constexpr static auto num = N;
	
	constexpr static auto den = D;
};

//////////////////////////////////////////////////////////////////////////////////////////

template <typename R>
struct Simplify
{
    constexpr static auto gcd = std::gcd(R::num, R::den);

    constexpr static auto num = R::num / gcd;

    constexpr static auto den = R::den / gcd;

    using type = Ratio<num, den>;
};

template <typename R>
using simplify = typename Simplify<R>::type;

//////////////////////////////////////////////////////////////////////////////////////////

template < typename R1, typename R2 > struct Sum
{
	constexpr static auto num = R1::num * R2::den + R2::num * R1::den;

	constexpr static auto den = R1::den * R2::den;

//  ------------------------------------------------------------------
	
	using type = simplify<Ratio<num, den>>;
};

template < typename R1, typename R2 > using sum = typename Sum < R1, R2 > ::type;

//////////////////////////////////////////////////////////////////////////////////////////

template <typename R1, typename R2>
struct Sub
{
    using type = sum<R1, Ratio<-R2::num, R2::den>>;
};

template <typename R1, typename R2>
using sub = typename Sub<R1, R2>::type;

//////////////////////////////////////////////////////////////////////////////////////////

template <typename R1, typename R2>
struct Mul
{
    constexpr static auto num = R1::num * R2::num;

    constexpr static auto den = R1::den * R2::den;

    using type = simplify<Ratio<num, den>>;
};

template <typename R1, typename R2>
using mul = typename Mul<R1, R2>::type;

//////////////////////////////////////////////////////////////////////////////////////////

template <typename R1, typename R2>
struct Div
{
    static_assert(R2::num != 0, "div by zero");

    using type = mul<R1, Ratio<R2::den, R2::num>>;
};

template <typename R1, typename R2>
using div = typename Div<R1, R2>::type;

//////////////////////////////////////////////////////////////////////////////////////////

template < typename T, typename R = Ratio < 1 > > struct Duration 
{
	T x = T();
};

//////////////////////////////////////////////////////////////////////////////////////////

template 
< 
	typename T1, typename R1, 
	
	typename T2, typename R2 
>
constexpr auto operator+(Duration < T1, R1 > const & lhs, Duration < T2, R2 > const & rhs)
{
	using ratio_t = Ratio < 1, sum < R1, R2 > ::den > ;

	auto x = 
	(
		lhs.x * ratio_t::den / R1::den * R1::num +

		rhs.x * ratio_t::den / R2::den * R2::num
	);

	return Duration < decltype(x), ratio_t > (x);
}

//////////////////////////////////////////////////////////////////////////////////////////

template
<
    typename T1, typename R1,

    typename T2, typename R2
>
constexpr auto operator-(Duration<T1, R1> const& lhs, Duration<T2, R2> const& rhs)
{
    return lhs + Duration<T2, Ratio<-R2::num, R2::den>>{rhs.x};
}

//////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    using R1 = Ratio<1, 2>;
    using R2 = Ratio<1, 3>;

    using SumResult = sum<R1, R2>;
    static_assert(SumResult::num == 5);
    static_assert(SumResult::den == 6);

    using SubResult = sub<R1, R2>;
    static_assert(SubResult::num == 1);
    static_assert(SubResult::den == 6);

    using MulResult = mul<R1, R2>;
    static_assert(MulResult::num == 1);
    static_assert(MulResult::den == 6);

    using DivResult = div<R1, R2>;
    static_assert(DivResult::num == 3);
    static_assert(DivResult::den == 2);

    using R3 = Ratio<2, 4>;
    using Simplified = simplify<R3>;
    static_assert(Simplified::num == 1);
    static_assert(Simplified::den == 2);

    using R4 = Ratio<4, 8>;
    using MulR3R4 = mul<R3, R4>;
    static_assert(MulR3R4::num == 1);
    static_assert(MulR3R4::den == 4);

    // duration
    auto x = 1, y = 2;

    Duration<int, Ratio<1, 2>> duration_1{x};
    Duration<int, Ratio<1, 3>> duration_2{y};

    auto duration_sum = duration_1 + duration_2;
    assert(duration_sum.x == 7);

    auto duration_sub = duration_1 - duration_2;
    assert(duration_sub.x == -1);
}

//////////////////////////////////////////////////////////////////////////////////////////