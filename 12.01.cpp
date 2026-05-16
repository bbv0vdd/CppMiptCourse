#include <cassert>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace StringUtils
{
    bool isSpace(char value)
    {
        return std::isspace(static_cast<unsigned char>(value)) != 0;
    }

    std::string ensureTrailingSpace(const std::string& text)
    {
        std::string result = text;
        if (result.empty() || !isSpace(result.back()))
        {
            result.push_back(' ');
        }
        return result;
    }
}

namespace CurrencyExchange
{
    class Converter
    {
    public:
        Converter(long double rubPerUsd, std::locale rubLocale, std::locale usdLocale)
            : m_rubPerUsd(rubPerUsd)
            , m_rubLocale(std::move(rubLocale))
            , m_usdLocale(std::move(usdLocale))
        {
            if (m_rubPerUsd <= 0.0L)
            {
                throw std::invalid_argument("Bad exchange rate");
            }
        }

        std::string convert(const std::string& rubAmount) const
        {
            const long double rubMinor = parseRubMinorUnits(rubAmount);
            const long long usdMinor = toUsdMinorUnits(rubMinor);
            return formatUsdMinorUnits(usdMinor);
        }

    private:
        long double parseRubMinorUnits(const std::string& rubText) const
        {
            std::stringstream stream(StringUtils::ensureTrailingSpace(rubText));
            stream.imbue(m_rubLocale);

            long double rubMinor{};
            stream >> std::showbase >> std::get_money(rubMinor, true);

            if (!stream)
            {
                throw std::invalid_argument("Bad RUB amount");
            }

            stream >> std::ws;
            if (!stream.eof())
            {
                throw std::invalid_argument("Bad RUB suffix");
            }

            return rubMinor;
        }

        long long toUsdMinorUnits(long double rubMinor) const
        {
            return static_cast<long long>(std::llround(rubMinor / m_rubPerUsd));
        }

        std::string formatUsdMinorUnits(long long usdMinor) const
        {
            std::stringstream stream;
            stream.imbue(m_usdLocale);
            stream << std::showbase << std::put_money(static_cast<long double>(usdMinor), false);

            if (!stream)
            {
                throw std::runtime_error("Bad USD output");
            }

            return stream.str();
        }

        long double m_rubPerUsd;
        std::locale m_rubLocale;
        std::locale m_usdLocale;
    };
}

namespace Tests
{
    void run(const CurrencyExchange::Converter& converter)
    {
        assert(converter.convert("45,00 RUB") == "$.61");
        assert(converter.convert("74,00 RUB") == "$1.00");
        assert(converter.convert("148,00 RUB") == "$2.00");
        assert(converter.convert("1234,80 RUB") == "$16.69");
    }

    void demo(const CurrencyExchange::Converter& converter)
    {
        const std::string input = "12345,67 RUB";
        const std::string output = converter.convert(input);
        std::cout << input << " -> " << output << '\n';
    }
}

int main()
{
    constexpr long double RUB_PER_USD = 74.0L;
    const std::locale RUB_LOCALE("ru_RU.utf8");
    const std::locale USD_LOCALE("en_US.utf8");

    const CurrencyExchange::Converter converter(RUB_PER_USD, RUB_LOCALE, USD_LOCALE);

    Tests::run(converter);
    Tests::demo(converter);

    return 0;
}