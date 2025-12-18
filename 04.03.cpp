/*
g++ -std=c++23 -Wall -Wextra 04.03.cpp -o 04.03.out
./04.03.out
*/

#include <cassert>
#include <vector>
#include <type_traits>

template <typename Container>
void handle(Container& container, int value)
{
    container.push_back(value);
}

template <typename Container, typename T>
void handle([[maybe_unused]] Container& container, [[maybe_unused]] T&& value)
{
}

template <typename Container, typename... Args>
void insert_pack(Container& container, Args&&... args)
{
    (handle(container, args), ...);
}


int main()
{
    {
        std::vector<int> vec;
        
        insert_pack(vec, 1, 2, 3, 4, 5);
        
        assert(vec.size() == 5);
        assert(vec[0] == 1);
        assert(vec[1] == 2);
        assert(vec[2] == 3);
        assert(vec[3] == 4);
        assert(vec[4] == 5);
    }
    
    {
        std::vector<int> vec;
        
        insert_pack(vec);
        
        assert(vec.empty());
    }

    {
        std::vector<int> vec;
        
        insert_pack(vec, 42, 3.14, "hello", 'a', 100, true);
        
        assert(vec.size() == 2);
        assert(vec[0] == 42);
        assert(vec[1] == 100);
    }

    return 0;
}
