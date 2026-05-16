#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <deque>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

template <typename Iter, typename Comp>
requires std::random_access_iterator<Iter>
void insertion(Iter first, Iter last, Comp comp)
{
    if (first == last) return;
    
    for (Iter cur = first + 1; cur != last; ++cur)
    {
        for (Iter it = cur; it != first; --it)
        {
            Iter prev = it - 1;
            if (!comp(*it, *prev)) break;
            std::iter_swap(prev, it);
        }
    }
}

template <typename Iter, typename Comp>
requires std::random_access_iterator<Iter>
Iter partition(Iter first, Iter last, Comp comp)
{
    Iter mid = first + (last - first) / 2;
    Iter last_elem = last - 1;
    
    if (comp(*mid, *first)) std::iter_swap(mid, first);
    if (comp(*last_elem, *first)) std::iter_swap(last_elem, first);
    if (comp(*last_elem, *mid)) std::iter_swap(last_elem, mid);
    
    auto pivot_val = *mid;
    Iter left = first;
    Iter right = last_elem;
    
    while (true)
    {
        while (comp(*left, pivot_val)) ++left;
        while (comp(pivot_val, *right)) --right;
        if (!(left < right)) return right + 1;
        std::iter_swap(left, right);
        ++left;
        --right;
    }
}

template <typename Iter, typename Comp>
requires std::random_access_iterator<Iter>
void quicksort(Iter first, Iter last, Comp comp)
{
    const std::ptrdiff_t threshold = 16;
    
    if (last - first > threshold)
    {
        Iter sep = partition(first, last, comp);
        quicksort(first, sep, comp);
        quicksort(sep, last, comp);
    }
    else
    {
        insertion(first, last, comp);
    }
}

template <typename Iter, typename Comp>
requires std::random_access_iterator<Iter>
void custom_sort(Iter first, Iter last, Comp comp)
{
    quicksort(first, last, comp);
}

bool desc(int a, int b)
{
    return a > b;
}

int main()
{
    {
        std::vector<int> v(1000);
        for (std::size_t i = 0; i < 1000; ++i) v[i] = 1000 - i;
        custom_sort(v.begin(), v.end(), std::less<int>{});
        assert(std::ranges::is_sorted(v, std::less<int>{}));
    }
    
    {
        std::vector<int> v(1000, 10);
        custom_sort(v.begin(), v.end(), std::less<int>{});
        assert(std::ranges::is_sorted(v, std::less<int>{}));
    }
    
    {
        std::vector<double> v(1000);
        for (std::size_t i = 0; i < 1000; ++i) v[i] = 1000.0 - i;
        custom_sort(v.begin(), v.end(), std::less<double>{});
        assert(std::ranges::is_sorted(v, std::less<double>{}));
    }
    
    {
        std::deque<int> d = { 4, 9, 2, 7, 0, 5, 8, 3, 6, 1 };
        custom_sort(d.begin(), d.end(), std::less<int>{});
        assert(std::ranges::is_sorted(d, std::less<int>{}));
    }
    
    {
        std::vector<int> v = {2, 1};
        custom_sort(v.begin(), v.end(), std::less<int>{});
        assert(std::ranges::is_sorted(v, std::less<int>{}));
    }
    
    {
        std::vector<int> v = {1};
        custom_sort(v.begin(), v.end(), std::less<int>{});
        assert(std::ranges::is_sorted(v, std::less<int>{}));
    }
    
    {
        std::vector<int> v = {};
        custom_sort(v.begin(), v.end(), std::less<int>{});
        assert(std::ranges::is_sorted(v, std::less<int>{}));
    }
    
    {
        std::vector<int> v = { 4, 9, 2, 7, 0, 5, 8, 3, 6, 1 };
        custom_sort(v.begin(), v.end(), desc);
        assert(std::ranges::is_sorted(v, desc));
    }
    
    {
        std::vector<int> v = { 4, 9, 2, 7, 0, 5, 8, 3, 6, 1 };
        auto lambda = [](int a, int b) { return a > b; };
        custom_sort(v.begin(), v.end(), lambda);
        assert(std::ranges::is_sorted(v, lambda));
    }
}

// score is 9/10
