/*
g++ -std=c++23 -Wall -Wextra 04.04.cpp -o 04.04.out
./04.04.out
*/

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <utility>

template <typename T>
class Vector
{
public:

    Vector() : m_array(nullptr), m_size(0), m_capacity(0)
    {
        std::cout << ("Vector::Vector (default)\n");
    }

    Vector(std::initializer_list<T> list)
    : m_size(std::size(list)),
      m_capacity(std::size(list))
    {
        std::cout << ("Vector::Vector (init-list)\n");
        m_array = m_size ? new T[m_size]{} : nullptr;
        std::ranges::copy(list, m_array);
    }

    Vector(Vector const & other)
    : m_array(nullptr),
      m_size(other.m_size),
      m_capacity(other.m_size)
    {
        std::cout << ("Vector::Vector (copy)\n");

        if (m_capacity != 0uz)
        {
            m_array = new T[m_capacity]{};
            std::copy_n(other.m_array, m_size, m_array);
        }
    }

    Vector(Vector && other) 
    : m_array(std::exchange(other.m_array, nullptr)),
      m_size(std::exchange(other.m_size, 0uz)),
      m_capacity(std::exchange(other.m_capacity, 0uz))
    {
        std::cout << ("Vector::Vector (move)\n");
    }

    ~Vector()
    {
        std::cout << ("Vector::~Vector\n");
        delete[] m_array;
    }

    Vector & operator=(Vector other)
    {
        std::cout << ("Vector::operator=\n");
        swap(other);
        return *this;
    }

    void swap(Vector & other) 
    {
        std::swap(m_array, other.m_array);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    std::size_t capacity() const 
    {
        return m_capacity;
    }

    std::size_t size() const 
    {
        return m_size;
    }

    bool empty() const 
    {
        return m_size == 0uz;
    }

    void clear() 
    {
        m_size = 0uz;
    }

    void push_back(T value)
    {
        grant_capacity_for_one_more();
        m_array[m_size++] = std::move(value);
    }

private:

    void grant_capacity_for_one_more()
    {
        if (m_size < m_capacity)
            return;

        std::size_t new_capacity = m_capacity ? m_capacity * 2 : 1;

        T * new_array = new T[new_capacity]{};

        if (m_size != 0uz)
            std::copy_n(m_array, m_size, new_array);

        delete[] m_array;
        m_array = new_array;
        m_capacity = new_capacity;
    }

private:

    T * m_array;
    std::size_t m_size;
    std::size_t m_capacity;
};

template <typename T>
void swap(Vector<T> & lhs, Vector<T> & rhs)
{
    lhs.swap(rhs);
}

int main()
{
    {
        Vector<int> v0;
        assert(v0.size() == 0uz);
        assert(v0.capacity() == 0uz);
        assert(v0.empty());
    }

    {
        Vector<int> v1 = {1, 2, 3, 4, 5};
        assert(v1.size() == 5uz);
        assert(v1.capacity() == 5uz);
        assert(!v1.empty());
        v1.clear();
        v1.push_back(10);
    }

    {
        Vector<int> v1 = {10};
        Vector<int> v2 = v1;
        assert(v2.size() == 1uz);
        assert(v2.capacity() == 1uz);
    }

    {
        Vector<int> v1 = {10};
        Vector<int> v2 = v1;
        Vector<int> v3 = std::move(v2);
        assert(v3.size() == 1uz);
        assert(v3.capacity() == 1uz);
        assert(v2.size() == 0uz);
        assert(v2.capacity() == 0uz);
    }

    {
        Vector<int> v4;
        std::size_t expected_capacity = 0uz;
        for (int i = 0; i < 10; ++i)
        {
            v4.push_back(i);
            if (expected_capacity == 0uz)
                expected_capacity = 1uz;
            while (expected_capacity <= static_cast<std::size_t>(i))
                expected_capacity *= 2;
            assert(v4.capacity() == expected_capacity);
            assert(v4.size() == static_cast<std::size_t>(i + 1));
        }
        assert(v4.capacity() == 16uz);

        auto cap_before_clear = v4.capacity();
        v4.clear();
        assert(v4.size() == 0uz);
        assert(v4.capacity() == cap_before_clear);
        assert(v4.empty());

        v4.push_back(42);
        assert(v4.size() == 1uz);
        assert(v4.capacity() == cap_before_clear);
    }

    {
        Vector<int> v1 = {10};
        Vector<int> v5;
        v5 = v1;
        assert(v5.size() == 1uz);
        assert(v5.capacity() == 1uz);
    }

    {
        Vector<int> v6;
        v6 = Vector<int>{7, 8, 9};
        assert(v6.size() == 3uz);
        assert(v6.capacity() == 3uz);
    }

    {
        Vector<int> v5 = {10};
        Vector<int> v6 = {7, 8, 9};
        swap(v5, v6);
        assert(v5.size() == 3uz);
        assert(v6.size() == 1uz);
    }

    // double
    {
        Vector<double> v0;
        assert(v0.size() == 0uz);
        assert(v0.capacity() == 0uz);
        assert(v0.empty());
    }

    {
        Vector<double> v1 = {1.1, 2.2, 3.3, 4.4, 5.5};
        assert(v1.size() == 5uz);
        assert(v1.capacity() == 5uz);
        assert(!v1.empty());
        v1.clear();
        v1.push_back(10.5);
    }

    {
        Vector<double> v1 = {10.5};
        Vector<double> v2 = v1;
        assert(v2.size() == 1uz);
        assert(v2.capacity() == 1uz);
    }

    {
        Vector<double> v1 = {10.5};
        Vector<double> v2 = v1;
        Vector<double> v3 = std::move(v2);
        assert(v3.size() == 1uz);
        assert(v3.capacity() == 1uz);
        assert(v2.size() == 0uz);
        assert(v2.capacity() == 0uz);
    }

    {
        Vector<double> v4;
        std::size_t expected_capacity = 0uz;
        for (int i = 0; i < 10; ++i)
        {
            v4.push_back(static_cast<double>(i) + 0.1);
            if (expected_capacity == 0uz)
                expected_capacity = 1uz;
            while (expected_capacity <= static_cast<std::size_t>(i))
                expected_capacity *= 2;
            assert(v4.capacity() == expected_capacity);
            assert(v4.size() == static_cast<std::size_t>(i + 1));
        }
        assert(v4.capacity() == 16uz);

        auto cap_before_clear = v4.capacity();
        v4.clear();
        assert(v4.size() == 0uz);
        assert(v4.capacity() == cap_before_clear);
        assert(v4.empty());

        v4.push_back(42.7);
        assert(v4.size() == 1uz);
        assert(v4.capacity() == cap_before_clear);
    }

    {
        Vector<double> v1 = {10.5};
        Vector<double> v5;
        v5 = v1;
        assert(v5.size() == 1uz);
        assert(v5.capacity() == 1uz);
    }

    {
        Vector<double> v6;
        v6 = Vector<double>{7.1, 8.2, 9.3};
        assert(v6.size() == 3uz);
        assert(v6.capacity() == 3uz);
    }

    {
        Vector<double> v5 = {10.5};
        Vector<double> v6 = {7.1, 8.2, 9.3};
        swap(v5, v6);
        assert(v5.size() == 3uz);
        assert(v6.size() == 1uz);
    }

    return 0;
}