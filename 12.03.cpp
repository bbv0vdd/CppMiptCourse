#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <vector>

namespace Palindrome
{
    class Cache
    {
    public:
        explicit Cache(std::size_t size)
            : m_size(size)
            , m_values(size * size, false)
        {
        }

        [[nodiscard]] bool get(std::size_t row, std::size_t col) const
        {
            return m_values[index(row, col)];
        }

        void set(std::size_t row, std::size_t col, bool value)
        {
            m_values[index(row, col)] = value;
        }

    private:
        [[nodiscard]] std::size_t index(std::size_t row, std::size_t col) const
        {
            return row * m_size + col;
        }

        std::size_t m_size;
        std::vector<bool> m_values;
    };

    [[nodiscard]] std::string_view findLongest(std::string_view text)
    {
        const std::size_t n = text.size();

        if (text.empty())
        {
            return {};
        }

        Cache cache(n);

        std::size_t bestStart = 0;
        std::size_t bestLen = 1;

        for (std::size_t i = 0; i < n; ++i)
        {
            cache.set(i, i, true);
        }

        for (std::size_t len = 2; len <= n; ++len)
        {
            for (std::size_t start = 0; start + len <= n; ++start)
            {
                const std::size_t end = start + len - 1;
                const bool edgesMatch = text[start] == text[end];
                const bool middleIsPalindrome = (len == 2) || cache.get(start + 1, end - 1);

                if (edgesMatch && middleIsPalindrome)
                {
                    cache.set(start, end, true);

                    if (len > bestLen)
                    {
                        bestStart = start;
                        bestLen = len;
                    }
                }
            }
        }

        return text.substr(bestStart, bestLen);
    }
}

namespace Tests
{
    void run()
    {
        using namespace Palindrome;

        assert(findLongest("") == "");
        assert(findLongest("a") == "a");
        assert(findLongest("aa") == "aa");
        assert(findLongest("ab") == "a");
        assert(findLongest("abba") == "abba");
        assert(findLongest("babad") == "bab");
        assert(findLongest("cbbd") == "bb");
        assert(findLongest("bananas") == "anana");
        assert(findLongest("forgeeksskeegfor") == "geeksskeeg");
        assert(findLongest("abacdfgdcaba") == "aba");
    }

    void demo()
    {
        const std::array<std::string_view, 3> examples = {"racecar", "babad", "cbbd"};

        for (const auto& ex : examples)
        {
            std::cout << ex << " -> " << Palindrome::findLongest(ex) << '\n';
        }
    }
}

int main()
{
    Tests::run();
    Tests::demo();
}