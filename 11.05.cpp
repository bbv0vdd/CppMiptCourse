#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

namespace TransformIf
{
    template <std::ranges::input_range R, typename Output, typename Predicate, typename Operation>
    Output apply(R&& range, Output output, Predicate predicate, Operation operation)
    {
        std::vector<std::ranges::range_value_t<R>> filtered;

        if constexpr (std::ranges::sized_range<R>)
        {
            filtered.reserve(static_cast<std::size_t>(std::ranges::size(range)));
        }

        std::ranges::copy_if(range, std::back_inserter(filtered), predicate);
        const auto result = std::ranges::transform(filtered, output, operation);

        return result.out;
    }
}

namespace VectorHelper
{
    template <std::ranges::input_range R>
    auto fromRange(R&& range)
    {
        std::vector<std::ranges::range_value_t<R>> values;

        if constexpr (std::ranges::sized_range<R>)
        {
            values.reserve(static_cast<std::size_t>(std::ranges::size(range)));
        }

        std::ranges::copy(range, std::back_inserter(values));
        return values;
    }
}

namespace ErrorMetrics
{
    constexpr double TOLERANCE = 1.0e-12;

    bool isNear(double first, double second)
    {
        return std::abs(first - second) < TOLERANCE;
    }

    template <std::ranges::forward_range First, std::ranges::forward_range Second>
    double meanAbsoluteError(const First& first, const Second& second)
    {
        const auto count = std::ranges::distance(first);
        assert(count > 0);
        assert(count == std::ranges::distance(second));

        const double sum = std::transform_reduce(
            std::ranges::begin(first),
            std::ranges::end(first),
            std::ranges::begin(second),
            0.0,
            std::plus<double>(),
            [](const auto left, const auto right)
            {
                return std::abs(static_cast<double>(left) - static_cast<double>(right));
            }
        );

        return sum / static_cast<double>(count);
    }

    template <std::ranges::forward_range First, std::ranges::forward_range Second>
    double meanSquaredError(const First& first, const Second& second)
    {
        const auto count = std::ranges::distance(first);
        assert(count > 0);
        assert(count == std::ranges::distance(second));

        const double sum = std::transform_reduce(
            std::ranges::begin(first),
            std::ranges::end(first),
            std::ranges::begin(second),
            0.0,
            std::plus<double>(),
            [](const auto left, const auto right)
            {
                const double error = static_cast<double>(left) - static_cast<double>(right);
                return error * error;
            }
        );

        return sum / static_cast<double>(count);
    }
}

namespace FibonacciSequence
{
    class Fibonacci : public std::ranges::view_interface<Fibonacci>
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using iterator_concept = std::forward_iterator_tag;
            using value_type = int;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;
            explicit Iterator(std::size_t index) : m_index(index) {}

            int operator*() const
            {
                return m_previous;
            }

            Iterator& operator++()
            {
                advance();
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator copy = *this;
                advance();
                return copy;
            }

            friend bool operator==(const Iterator& first, const Iterator& second)
            {
                return first.m_index == second.m_index;
            }

        private:
            void advance()
            {
                const int next = m_previous + m_current;
                m_previous = m_current;
                m_current = next;
                ++m_index;
            }

            int m_previous = 0;
            int m_current = 1;
            std::size_t m_index = 0;
        };

        Fibonacci() = default;
        explicit Fibonacci(std::size_t count) : m_count(count) {}

        Iterator begin() const
        {
            return Iterator(0);
        }

        Iterator end() const
        {
            return Iterator(m_count);
        }

    private:
        std::size_t m_count = 0;
    };

    std::vector<int> make_v1(std::size_t count)
    {
        std::vector<int> values;
        values.reserve(count);

        for (auto it = Fibonacci(count).begin(); it != Fibonacci(count).end(); ++it)
        {
            values.push_back(*it);
        }

        return values;
    }

    std::vector<int> make_v2(std::size_t count)
    {
        std::vector<int> values;
        values.reserve(count);

        for (int value : Fibonacci(count))
        {
            values.push_back(value);
        }

        return values;
    }

    std::vector<int> make_v3(std::size_t count)
    {
        return VectorHelper::fromRange(Fibonacci(count));
    }
}

namespace Tests
{
    using namespace ErrorMetrics;
    using namespace TransformIf;

    void testReplace()
    {
        constexpr int old_val = 2;
        constexpr int new_val = 9;
        std::vector<int> values = {1, 2, 3, 2, 4};
        const std::vector<int> expected = {1, 9, 3, 9, 4};

        std::ranges::replace(values, old_val, new_val);
        assert(values == expected);
    }

    void testFill()
    {
        constexpr std::size_t count = 4;
        std::vector<int> values(count, 0);
        const std::vector<int> expected = {7, 7, 7, 7};

        std::ranges::fill(values, 7);
        assert(values == expected);
    }

    void testUnique()
    {
        std::vector<int> values = {1, 1, 2, 2, 2, 3, 3};
        const std::vector<int> expected = {1, 2, 3};

        const auto tail = std::ranges::unique(values);
        values.erase(tail.begin(), tail.end());
        assert(values == expected);
    }

    void testRotate()
    {
        std::vector<int> values = {1, 2, 3, 4, 5};
        const std::vector<int> expected = {3, 4, 5, 1, 2};

        std::ranges::rotate(values, std::next(values.begin(), 2));
        assert(values == expected);
    }

    void testSample()
    {
        constexpr std::size_t sample_size = 3;
        const std::vector<int> values = {1, 2, 3, 4, 5, 6};
        std::vector<int> sampled;
        std::mt19937 generator(123);

        std::ranges::sample(values, std::back_inserter(sampled), sample_size, generator);

        assert(sampled.size() == sample_size);
        for (int value : sampled)
        {
            assert(std::ranges::find(values, value) != values.end());
        }
    }

    void testTransformIf()
    {
        const std::vector<int> values = {1, 2, 3, 4, 5};
        const std::vector<int> expected = {20, 40};
        std::vector<int> result;

        const auto is_even = [](int v) { return v % 2 == 0; };
        const auto multiply = [](int v) { return v * 10; };

        apply(values, std::back_inserter(result), is_even, multiply);
        assert(result == expected);
    }

    void testErrors()
    {
        const std::vector<double> expected = {1.0, 2.0, 3.0, 4.0};
        const std::vector<double> actual = {1.0, 3.0, 2.0, 6.0};

        assert(isNear(meanAbsoluteError(expected, actual), 1.0));
        assert(isNear(meanSquaredError(expected, actual), 1.5));
    }

    void testFilter()
    {
        const std::vector<int> values = {1, 2, 3, 4, 5, 6};
        const std::vector<int> expected = {2, 4, 6};

        const auto is_even = [](int v) { return v % 2 == 0; };
        auto view = values | std::views::filter(is_even);

        assert(VectorHelper::fromRange(view) == expected);
    }

    void testDrop()
    {
        const std::vector<int> values = {1, 2, 3, 4, 5};
        const std::vector<int> expected = {3, 4, 5};

        auto view = values | std::views::drop(2);
        assert(VectorHelper::fromRange(view) == expected);
    }

    void testJoin()
    {
        const std::vector<std::vector<int>> values = {{1, 2}, {3}, {4, 5}};
        const std::vector<int> expected = {1, 2, 3, 4, 5};

        auto view = values | std::views::join;
        assert(VectorHelper::fromRange(view) == expected);
    }

    void testZip()
    {
        const std::vector<int> first = {1, 2, 3};
        const std::vector<int> second = {4, 5, 6};
        const std::vector<int> expected = {5, 7, 9};
        std::vector<int> sums;

        for (const auto [l, r] : std::views::zip(first, second))
        {
            sums.push_back(l + r);
        }

        assert(sums == expected);
    }

    void testStride()
    {
        const std::vector<int> values = {1, 2, 3, 4, 5, 6};
        const std::vector<int> expected = {1, 3, 5};

        auto view = values | std::views::stride(2);
        assert(VectorHelper::fromRange(view) == expected);
    }

    void testFibonacci()
    {
        using namespace FibonacciSequence;

        static_assert(std::ranges::view<Fibonacci>);
        static_assert(std::ranges::forward_range<Fibonacci>);

        constexpr std::size_t empty = 0, one = 1, basic = 8;
        const std::vector<int> one_expected = {0};
        const std::vector<int> basic_expected = {0, 1, 1, 2, 3, 5, 8, 13};
        const std::vector<int> view_expected = {2, 3, 5, 8};

        assert(make_v1(empty).empty());
        assert(make_v1(one) == one_expected);
        assert(make_v1(basic) == basic_expected);
        assert(make_v2(basic) == basic_expected);
        assert(make_v3(basic) == basic_expected);

        auto view = Fibonacci(basic) | std::views::drop(3) | std::views::take(4);
        assert(VectorHelper::fromRange(view) == view_expected);

        auto it = Fibonacci(basic).begin();
        assert(*it == 0);
        ++it;
        assert(*it == 1);
        it++;
        assert(*it == 1);
        ++it;
        assert(*it == 2);
    }

    void runAll()
    {
        testReplace();
        testFill();
        testUnique();
        testRotate();
        testSample();
        testTransformIf();
        testErrors();
        testFilter();
        testDrop();
        testJoin();
        testZip();
        testStride();
        testFibonacci();
    }
}

int main()
{
    Tests::runAll();
    return 0;
}