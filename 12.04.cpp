#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <regex>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

namespace EmailParser
{
    class Entry
    {
    public:
        Entry(std::string address, std::string domain)
            : m_address(std::move(address))
            , m_domain(std::move(domain))
        {
        }

        [[nodiscard]] const std::string& address() const
        {
            return m_address;
        }

        [[nodiscard]] const std::string& domain() const
        {
            return m_domain;
        }

        [[nodiscard]] bool operator==(const Entry& other) const
        {
            return m_address == other.m_address && m_domain == other.m_domain;
        }

    private:
        std::string m_address;
        std::string m_domain;
    };

    using EntryList = std::vector<Entry>;

    [[nodiscard]] EntryList extract(const std::string& text)
    {
        constexpr int fullEmailIdx = 0;
        constexpr int domainIdx = 1;
        constexpr std::size_t domainOffset = 1;
        constexpr std::size_t pairSize = 2;

        std::regex pattern(
            R"([a-z0-9._%+-]+@([a-z0-9.-]+\.[a-z]{2,}))",
            std::regex_constants::icase
        );

        std::vector<std::string> tokens;

        auto collector = [&tokens](const auto& match)
        {
            tokens.push_back(match.str());
        };

        std::ranges::for_each(
            std::sregex_token_iterator(
                std::cbegin(text),
                std::cend(text),
                pattern,
                {fullEmailIdx, domainIdx}
            ),
            std::sregex_token_iterator(),
            collector
        );

        assert(tokens.size() % pairSize == 0);

        EntryList entries;
        entries.reserve(tokens.size() / pairSize);

        for (std::size_t i = 0; i < tokens.size(); i += pairSize)
        {
            entries.emplace_back(tokens.at(i), tokens.at(i + domainOffset));
        }

        return entries;
    }
}

namespace Tests
{
    using namespace EmailParser;

    void check(const std::string& text, const EntryList& expected)
    {
        const auto actual = extract(text);
        assert(actual == expected);
    }

    void run()
    {
        const auto text = R"(Contacts:
first.last@example.com
support+shop@sub.example.org, admin@mail-server.net.
Not email: username@localhost and @broken.com
)"s;

        check(text, EntryList({
            Entry("first.last@example.com", "example.com"),
            Entry("support+shop@sub.example.org", "sub.example.org"),
            Entry("admin@mail-server.net", "mail-server.net")
        }));

        const auto uppercase = R"(Write to USER.NAME@EXAMPLE.COM or Root@Host.Co.Uk.)"s;

        check(uppercase, EntryList({
            Entry("USER.NAME@EXAMPLE.COM", "EXAMPLE.COM"),
            Entry("Root@Host.Co.Uk", "Host.Co.Uk")
        }));

        const auto empty = R"(There are no addresses here.)"s;
        check(empty, EntryList());
    }
}

int main()
{
    Tests::run();
}