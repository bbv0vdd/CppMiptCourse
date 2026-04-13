#include <cassert>
#include <cstddef>
#include <new>
#include <print>
#include <stdexcept>

template <typename T>
class Allocator
{
public:
    static void* operator new(std::size_t size)
    {
        std::print("Allocator::new, size = {}\n", size);
        return ::operator new(size);
    }

    static void operator delete(void* ptr)
    {
        std::print("Allocator::delete\n");
        ::operator delete(ptr);
    }

    static void* operator new[](std::size_t size)
    {
        std::print("Allocator::new[], size = {}\n", size);
        return ::operator new[](size);
    }

    static void operator delete[](void* ptr)
    {
        std::print("Allocator::delete[]\n");
        ::operator delete[](ptr);
    }

    static void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept
    {
        std::print("Allocator::new(nothrow), size = {}\n", size);
        return ::operator new(size, tag);
    }

    static void operator delete(void* ptr, const std::nothrow_t& tag) noexcept
    {
        std::print("Allocator::delete(nothrow)\n");
        ::operator delete(ptr, tag);
    }

    static void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept
    {
        std::print("Allocator::new[](nothrow), size = {}\n", size);
        return ::operator new[](size, tag);
    }

    static void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept
    {
        std::print("Allocator::delete[](nothrow)\n");
        ::operator delete[](ptr, tag);
    }

protected:
    Allocator() = default;
};

class User : private Allocator<User>
{
public:
    User()
        : m_value(0)
    {
        std::print("User::User, this = {}\n", static_cast<const void*>(this));

        if (s_failFlag)
        {
            throw std::runtime_error("User construction failed");
        }
    }

    ~User()
    {
        std::print("User::~User, this = {}\n", static_cast<const void*>(this));
    }

    void set_value(int val) noexcept
    {
        m_value = val;
    }

    int get_value() const noexcept
    {
        return m_value;
    }

    static void enable_failure(bool enable) noexcept
    {
        s_failFlag = enable;
    }

    using Allocator<User>::operator new;
    using Allocator<User>::operator delete;
    using Allocator<User>::operator new[];
    using Allocator<User>::operator delete[];

private:
    static inline bool s_failFlag = false;
    int m_value;
};

int main()
{
    constexpr std::size_t COUNT = 3;

    std::print("single:\n\n");
    {
        User* p = new User;
        assert(p);
        p->set_value(100);
        assert(p->get_value() == 100);
        delete p;
    }

    std::print("array:\n\n");
    {
        User* arr = new User[COUNT];
        assert(arr);
        for (std::size_t i = 0; i < COUNT; ++i)
        {
            arr[i].set_value(static_cast<int>(i * 10));
            assert(arr[i].get_value() == static_cast<int>(i * 10));
        }
        delete[] arr;
    }

    std::print("single nothrow:\n\n");
    {
        User* p = new (std::nothrow) User;
        assert(p);
        delete p;
    }

    std::print("array nothrow:\n\n");
    {
        User* arr = new (std::nothrow) User[COUNT];
        assert(arr);
        delete[] arr;
    }

    std::print("single fail:\n\n");
    {
        User::enable_failure(true);

        try
        {
            (void)new (std::nothrow) User;
            assert(false);
        }
        catch (const std::runtime_error&)
        {
            std::print("Single fail caught\n");
        }

        User::enable_failure(false);
    }

    std::print("array fail: \n\n");
    {
        User::enable_failure(true);

        try
        {
            (void)new (std::nothrow) User[COUNT];
            assert(false);
        }
        catch (const std::runtime_error&)
        {
            std::print("Array fail caught\n");
        }

        User::enable_failure(false);
    }

    std::print("\nAll tests passed\n");
}
