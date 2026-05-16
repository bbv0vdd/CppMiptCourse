/*
g++ -std=c++23 -Wall -Wextra -Wpedantic 09.04.cpp -o 09.04.out
./09.04.out
*/

#include <cassert>
#include <concepts>
#include <cstddef>
#include <deque>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>
#include <algorithm>

template < typename Iter >
requires std::random_access_iterator < Iter >
void insertion(Iter start, Iter end)
{
    if (start == end) return;
    
    for (Iter cur = start + 1; cur != end; ++cur)
    {
        Iter pos = cur;
        while (pos != start && *pos < *(pos - 1))
        {
            std::iter_swap(pos, pos - 1);
            --pos;
        }
    }
}

template < typename Iter >
requires std::random_access_iterator < Iter >
Iter partition(Iter left, Iter right)
{
    Iter mid = left + (right - left) / 2;
    Iter last = right - 1;
    
    if (*mid < *left) std::iter_swap(mid, left);
    if (*last < *left) std::iter_swap(last, left);
    if (*last < *mid) std::iter_swap(last, mid);
    
    auto pivot_val = *mid;
    
    Iter i = left;
    Iter j = last;
    
    while (true)
    {
        while (*i < pivot_val) ++i;
        while (pivot_val < *j) --j;
        if (i >= j) return j + 1;
        std::iter_swap(i, j);
        ++i;
        --j;
    }
}

template < typename Iter >
requires std::random_access_iterator < Iter >
void qsort(Iter begin, Iter end)
{
    const std::ptrdiff_t threshold = 16;
    
    if (end - begin > threshold)
    {
        Iter split = partition(begin, end);
        qsort(begin, split);
        qsort(split, end);
    }
    else
    {
        insertion(begin, end);
    }
}

template < typename Iter >
requires std::random_access_iterator < Iter >
void sort(Iter first, Iter last)
{
    qsort(first, last);
}

int main()
{
    {
        const std::size_t sz = 1000U;
        std::vector < int > data(sz, 0);
        for (std::size_t idx = 0; idx < sz; ++idx)
            data[idx] = static_cast < int > (sz - idx);
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    {
        const std::size_t sz = 1000U;
        std::vector < int > data(sz, 10);
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    {
        const std::size_t sz = 1000U;
        std::vector < double > data(sz, 0.0);
        for (std::size_t idx = 0; idx < sz; ++idx)
            data[idx] = static_cast < double > (sz - idx);
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    {
        std::deque < int > data = { 4, 9, 2, 7, 0, 5, 8, 3, 6, 1 };
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    {
        std::vector < int > data = { 2, 1 };
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    {
        std::vector < int > data = { 1 };
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));   
    }
    
    {
        std::vector < int > data = { };
        sort(data.begin(), data.end());
        assert(std::ranges::is_sorted(data));
    }
    
    // return 0; for int main()'s
}

// Score is 9/10
