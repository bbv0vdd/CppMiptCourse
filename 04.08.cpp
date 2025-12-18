/*
g++ -std=c++23 -Wall -Wextra 04.08.cpp -o 04.08.out
./04.08.out
*/

//////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstddef>
#include <string>
#include <utility>
#include <type_traits>

//////////////////////////////////////////////////////////////////

template < typename ... Ts > class Tuple 
{
public:
    constexpr Tuple() = default;

    static constexpr std::size_t size() noexcept
    {
        return sizeof...(Ts);
    }
};

//////////////////////////////////////////////////////////////////

template < typename T, typename ... Ts > 
class Tuple < T, Ts ... >
{
public :
    
	constexpr Tuple(T && x, Ts && ... xs)
	:
		m_head(std::forward < T  > (x )   ),
		
		m_tail(std::forward < Ts > (xs)...)
	{}

    static constexpr std::size_t size() noexcept
    {
        return 1 + sizeof...(Ts);
    }


	template < std::size_t I > 
    constexpr auto get() const
	{
		if constexpr (I > 0)
		{
			return m_tail.template get < I - 1 > ();
		}
		else
		{
			return m_head;
		}
	}

private :

	T m_head;
	
	Tuple < Ts ... > m_tail;
};

//////////////////////////////////////////////////////////////////

int main()
{
	Tuple < int, double, std::string > tuple(1, 2.0, "aaaaa");
	assert(tuple.get < 0 > () == 1);

    {
        Tuple<> empty_tuple;
        static_assert(empty_tuple.size() == 0);
        static_assert(Tuple<>::size() == 0);
    }

    {
        constexpr Tuple<int> single_tuple(42);
        static_assert(single_tuple.size() == 1);
        static_assert(Tuple<int>::size() == 1);
        static_assert(single_tuple.get<0>() == 42);
    }

    {
        constexpr Tuple<int, double, char> multi_tuple(10, 3.14, 'A');
        static_assert(multi_tuple.size() == 3);
        static_assert(Tuple<int, double, char>::size() == 3);
        static_assert(multi_tuple.get<0>() == 10);
        static_assert(multi_tuple.get<1>() == 3.14);
        static_assert(multi_tuple.get<2>() == 'A');
    }

    {
        constexpr Tuple<int, Tuple<double, char>> nested_tuple(5, Tuple<double, char>(3.14, 'X'));
        static_assert(nested_tuple.size() == 2);
        static_assert(nested_tuple.get<0>() == 5);
        static_assert(nested_tuple.get<1>().size() == 2);
        static_assert(nested_tuple.get<1>().get<0>() == 3.14);
        static_assert(nested_tuple.get<1>().get<1>() == 'X');
    }
}

//////////////////////////////////////////////////////////////////