/*
g++ -std=c++23 -Wall -Wextra 09.01.cpp -o 09.01.out
./09.01.out
*/

#include <cassert>
#include <cstddef>
#include <iostream>
#include <source_location>
#include <type_traits>
#include <utility>

class Tracer
{
public:
    Tracer(const std::source_location location = std::source_location::current()) noexcept
        : m_location(location), m_depth(current_depth())
    {
        ++current_depth();
        print_enter();
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

    ~Tracer() noexcept
    {
        if (current_depth() > 0U)
        {
            --current_depth();
        }
        print_leave();
    }

private:
    static std::size_t& current_depth() noexcept
    {
        static std::size_t depth = 0U;
        return depth;
    }

    static void print_indent(const std::size_t depth)
    {
        constexpr const char* indent_unit = "  ";
        for (std::size_t index = 0U; index < depth; ++index)
        {
            std::cout << indent_unit;
        }
    }

    void print_enter() const
    {
        print_indent(m_depth);
        std::cout << "enter : " << m_location.function_name()
                  << " [" << m_location.file_name()
                  << ':' << m_location.line()
                  << ':' << m_location.column()
                  << "]\n";
    }

    void print_leave() const
    {
        print_indent(m_depth);
        std::cout << "leave : " << m_location.function_name()
                  << " [" << m_location.file_name()
                  << ':' << m_location.line()
                  << ':' << m_location.column()
                  << "]\n";
    }

private:
    std::source_location m_location{};
    std::size_t m_depth = 0U;
};



#ifndef NDEBUG
    #define trace() Tracer TRACE_CONCAT(m_tracer_, __LINE__)
    #define TRACE_CONCAT_IMPL(x, y) x##y
#define TRACE_CONCAT(x, y) TRACE_CONCAT_IMPL(x, y)
#else
    #define trace() ((void)0)
#endif

static int add(const int a, const int b)
{
    trace();

    return a + b;
}

static void run_tests()
{
    trace();
    static_assert(!std::is_copy_constructible_v<Tracer>, "Tracer must be non-copyable");
    static_assert(!std::is_copy_assignable_v<Tracer>, "Tracer must be non-copy-assignable");
    static_assert(!std::is_move_constructible_v<Tracer>, "Tracer must be non-movable");
    static_assert(!std::is_move_assignable_v<Tracer>, "Tracer must be non-move-assignable");
    assert(add(0, 1) == 1);
    assert(add(2, 5) == 7);
}

int main()
{
    trace();
    run_tests();
    std::cout << "All tests passed\n";
}