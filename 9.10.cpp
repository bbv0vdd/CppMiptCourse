/*
g++ -std=c++23 -Wall -Wextra -Wpedantic 09.10.cpp -o 09.10.out
./09.10.out
*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <utility>
#include <vector>

class Allocator
{
public:
    virtual ~Allocator() = default;
    virtual auto allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) -> void * = 0;
    virtual void deallocate(void * pointer, std::size_t size = 0) = 0;

protected:
    template <typename T>
    auto get(void * pointer) const -> T *
    {
        return static_cast<T *>(pointer);
    }

    template <typename T>
    auto get(void const * pointer) const -> T const *
    {
        return static_cast<T const *>(pointer);
    }
};

class LinearAllocator final : public Allocator
{
public:
    explicit LinearAllocator(std::size_t size) : m_size(size)
    {
        m_begin = ::operator new(m_size, std::align_val_t(s_alignment));
    }

    ~LinearAllocator() override
    {
        ::operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    LinearAllocator(LinearAllocator const &) = delete;
    auto operator=(LinearAllocator const &) -> LinearAllocator & = delete;

    auto allocate(std::size_t size, std::size_t alignment = s_alignment) -> void * override
    {
        void * begin = get<std::byte>(m_begin) + m_offset;
        std::size_t free = m_size - m_offset;

        if (std::align(alignment, size, begin, free) == nullptr)
            return nullptr;

        m_offset = m_size - free + size;
        return begin;
    }

    void deallocate(void *, std::size_t = 0) override {}

    void reset() { m_offset = 0; }
    void show(char const * name) const
    {
        std::cout << name << " : m_size = " << m_size << " m_begin = " << m_begin << " m_offset = " << m_offset << '\n';
    }

private:
    std::size_t m_size = 0;
    std::size_t m_offset = 0;
    void * m_begin = nullptr;
    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

class StackAllocator final : public Allocator
{
public:
    explicit StackAllocator(std::size_t size) : m_size(size)
    {
        m_begin = ::operator new(m_size, std::align_val_t(s_alignment));
    }

    ~StackAllocator() override
    {
        ::operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    StackAllocator(StackAllocator const &) = delete;
    auto operator=(StackAllocator const &) -> StackAllocator & = delete;

    auto allocate(std::size_t size, std::size_t alignment = s_alignment) -> void * override
    {
        void * begin = get<std::byte>(m_begin) + m_offset + sizeof(Header);
        std::size_t free = m_size - m_offset - sizeof(Header);

        if (std::align(alignment, size, begin, free) == nullptr)
            return nullptr;

        auto * header = get<Header>(get<std::byte>(begin) - sizeof(Header));
        header->m_padding = static_cast<std::size_t>(get<std::byte>(begin) - (get<std::byte>(m_begin) + m_offset));
        m_offset = static_cast<std::size_t>(get<std::byte>(begin) - get<std::byte>(m_begin)) + size;
        return begin;
    }

    void deallocate(void * pointer, std::size_t = 0) override
    {
        assert(pointer != nullptr);
        auto * header = get<Header>(get<std::byte>(pointer) - sizeof(Header));
        m_offset = static_cast<std::size_t>(get<std::byte>(pointer) - get<std::byte>(m_begin)) - header->m_padding;
    }

    void show(char const * name) const
    {
        std::cout << name << " : m_size = " << m_size << " m_begin = " << m_begin << " m_offset = " << m_offset << '\n';
    }

private:
    struct Header { std::size_t m_padding = 0; };
    std::size_t m_size = 0;
    std::size_t m_offset = 0;
    void * m_begin = nullptr;
    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

class ListAllocator final : public Allocator
{
public:
    ListAllocator(std::size_t size, std::size_t step) : m_size(size), m_step(step)
    {
        assert(m_size % m_step == 0);
        assert(m_step >= sizeof(Node));
        make_list();
        m_begin = m_head;
    }

    ~ListAllocator() override
    {
        for (void * list : m_lists)
            ::operator delete(list, m_size, std::align_val_t(s_alignment));
    }

    ListAllocator(ListAllocator const &) = delete;
    auto operator=(ListAllocator const &) -> ListAllocator & = delete;

    auto allocate(std::size_t size = 0, std::size_t alignment = s_alignment) -> void * override
    {
        if (size > m_step || alignment > s_alignment)
            return nullptr;

        if (m_head == nullptr)
        {
            if (m_offset == m_lists.size())
                make_list();
            else
                m_head = get<Node>(m_lists[++m_offset - 1]);
        }

        Node * node = m_head;
        m_head = (node->m_next == nullptr && get<std::byte>(node) + m_step != get<std::byte>(m_lists[m_offset - 1]) + m_size)
                 ? get<Node>(get<std::byte>(node) + m_step)
                 : node->m_next;
        
        if (m_head != nullptr && get<std::byte>(m_head) == get<std::byte>(node) + m_step)
            m_head->m_next = nullptr;

        return node;
    }

    void deallocate(void * pointer, std::size_t = 0) override
    {
        assert(pointer != nullptr);
        auto * node = get<Node>(pointer);
        node->m_next = m_head;
        m_head = node;
    }

    void show(char const * name) const
    {
        std::cout << name << " : m_size = " << m_size << " m_step = " << m_step
                  << " m_begin = " << m_begin << " m_head = " << static_cast<void *>(m_head)
                  << " m_offset = " << m_offset << '\n';
    }

private:
    struct Node { Node * m_next = nullptr; };
    
    void make_list()
    {
        m_head = get<Node>(::operator new(m_size, std::align_val_t(s_alignment)));
        m_head->m_next = nullptr;
        ++m_offset;
        m_lists.push_back(m_head);
    }

    std::size_t m_size = 0;
    std::size_t m_step = 0;
    std::size_t m_offset = 0;
    void * m_begin = nullptr;
    Node * m_head = nullptr;
    std::vector<void *> m_lists;
    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

class FreeListAllocator final : public Allocator
{
public:
    explicit FreeListAllocator(std::size_t size) : m_size(size)
    {
        assert(m_size >= sizeof(Node) + 1);
        m_begin = ::operator new(m_size, std::align_val_t(s_alignment));
        m_head = get<Node>(m_begin);
        m_head->m_size = m_size - sizeof(Header);
        m_head->m_next = nullptr;
    }

    ~FreeListAllocator() override
    {
        ::operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    FreeListAllocator(FreeListAllocator const &) = delete;
    auto operator=(FreeListAllocator const &) -> FreeListAllocator & = delete;

    auto allocate(std::size_t size, std::size_t alignment = alignof(Header)) -> void * override
    {
        void * end = get<std::byte>(m_begin) + sizeof(Header) + size;
        void * next = end;
        std::size_t free = 2 * alignof(Header);

        if (std::align(alignment, sizeof(Header), next, free) == nullptr)
            return nullptr;

        auto padding = static_cast<std::size_t>(get<std::byte>(next) - get<std::byte>(end));
        auto wanted = size + padding;
        auto found = find(wanted);
        Node * current = found.first;
        Node * previous = found.second;

        if (current == nullptr)
            return nullptr;

        if (current->m_size >= wanted + sizeof(Node) + 1)
        {
            auto step = sizeof(Header) + wanted;
            auto * node = get<Node>(get<std::byte>(current) + step);
            node->m_size = current->m_size - step;
            node->m_next = current->m_next;
            current->m_next = node;
        }
        else
        {
            padding += current->m_size - wanted;
            wanted = current->m_size;
        }

        if (previous == nullptr)
            m_head = current->m_next;
        else
            previous->m_next = current->m_next;

        auto * header = get<Header>(current);
        header->m_size = wanted;
        return get<std::byte>(current) + sizeof(Header);
    }

    void deallocate(void * pointer, std::size_t = 0) override
    {
        assert(pointer != nullptr);
        auto * node = get<Node>(get<std::byte>(pointer) - sizeof(Header));
        Node * previous = nullptr;
        Node * current = m_head;

        while (current != nullptr && node > current)
        {
            previous = current;
            current = current->m_next;
        }

        node->m_next = current;
        if (previous == nullptr)
            m_head = node;
        else
            previous->m_next = node;

        merge(previous, node);
    }

    void show(char const * name) const
    {
        std::cout << name << " : m_size = " << m_size << " m_begin = " << m_begin
                  << " m_head = " << static_cast<void *>(m_head);
        if (m_head != nullptr && m_head->m_next != nullptr)
            std::cout << " m_head->m_next = " << static_cast<void *>(m_head->m_next);
        std::cout << '\n';
    }

private:
    struct Node
    {
        std::size_t m_size = 0;
        Node * m_next = nullptr;
    };

    struct alignas(std::max_align_t) Header
    {
        std::size_t m_size = 0;
    };

    auto find(std::size_t size) const -> std::pair<Node *, Node *>
    {
        Node * current = m_head;
        Node * previous = nullptr;
        while (current != nullptr && size > current->m_size)
        {
            previous = current;
            current = current->m_next;
        }
        return {current, previous};
    }

    void merge(Node * previous, Node * node)
    {
        if (node->m_next != nullptr &&
            get<std::byte>(node) + sizeof(Header) + node->m_size == get<std::byte>(node->m_next))
        {
            node->m_size += sizeof(Header) + node->m_next->m_size;
            node->m_next = node->m_next->m_next;
        }

        if (previous != nullptr &&
            get<std::byte>(previous) + sizeof(Header) + previous->m_size == get<std::byte>(node))
        {
            previous->m_size += sizeof(Header) + node->m_size;
            previous->m_next = node->m_next;
        }
    }

    std::size_t m_size = 0;
    void * m_begin = nullptr;
    Node * m_head = nullptr;
    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

void test_linear_allocator()
{
    LinearAllocator allocator(1U << 10);
    allocator.show("LinearAllocator");

    void * a = allocator.allocate(1, 1);
    void * b = allocator.allocate(2, 2);
    void * c = allocator.allocate(4, 4);
    void * d = allocator.allocate(8, 8);
    assert(a && b && c && d);
    allocator.show("LinearAllocator");

    allocator.reset();
    void * e = allocator.allocate(16, 16);
    assert(e);
}

void test_stack_allocator()
{
    StackAllocator allocator(1U << 10);
    allocator.show("StackAllocator");

    allocator.allocate(1, 1);
    allocator.allocate(2, 2);
    void * x = allocator.allocate(4, 4);
    void * y = allocator.allocate(8, 8);
    assert(x && y);
    allocator.show("StackAllocator");

    allocator.deallocate(y);
    allocator.deallocate(x);
    void * z = allocator.allocate(4, 4);
    allocator.show("StackAllocator");
    assert(z == x);
}

void test_list_allocator()
{
    ListAllocator allocator(32, 8);
    allocator.show("ListAllocator");

    allocator.allocate(8);
    void * x = allocator.allocate(8);
    void * y = allocator.allocate(8);
    allocator.allocate(8);
    allocator.allocate(8);
    assert(x && y);
    allocator.show("ListAllocator");

    allocator.deallocate(x);
    allocator.deallocate(y);
    void * z = allocator.allocate(8);
    allocator.show("ListAllocator");
    assert(z == y);
}

void test_free_list_allocator()
{
    FreeListAllocator allocator(1U << 10);
    allocator.show("FreeListAllocator");

    allocator.allocate(16);
    void * x = allocator.allocate(16);
    void * y = allocator.allocate(16);
    allocator.allocate(16);
    assert(x && y);
    allocator.show("FreeListAllocator");

    allocator.deallocate(y);
    allocator.deallocate(x);
    void * z = allocator.allocate(32);
    allocator.show("FreeListAllocator");
    assert(z == x);
}

void demo_polymorphic_allocators()
{
    LinearAllocator linear(256);
    StackAllocator stack(256);
    ListAllocator list(64, 16);
    FreeListAllocator free_list(512);

    std::array<Allocator *, 4> allocators = {&linear, &stack, &list, &free_list};

    std::cout << "Polymorphic allocators\n";
    for (Allocator * allocator : allocators)
    {
        void * pointer = allocator->allocate(16, 16);
        std::cout << "pointer = " << pointer << '\n';
        assert(pointer);
        allocator->deallocate(pointer, 16);
    }
}

int main()
{
    test_linear_allocator();
    test_stack_allocator();
    test_list_allocator();
    test_free_list_allocator();
    std::cout << '\n';
    demo_polymorphic_allocators();
    std::cout << "\nAll tests passed\n";
    return 0;
}