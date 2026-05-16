#include <cassert>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace HexConverter
{
    constexpr std::size_t BYTE_HEX_WIDTH = 2;
    constexpr int NIBBLE_BITS = 4;
    constexpr std::uint8_t DECIMAL_DIGIT_COUNT = 10;

    std::string encode(const std::vector<std::uint8_t>& bytes)
    {
        std::stringstream ss;
        ss << std::hex << std::right << std::setfill('0');

        for (auto val : bytes)
        {
            ss << std::setw(BYTE_HEX_WIDTH) << static_cast<unsigned int>(val);
        }

        return ss.str();
    }

    std::uint8_t digitToValue(char digit)
    {
        if (digit >= '0' && digit <= '9')
        {
            return static_cast<std::uint8_t>(digit - '0');
        }

        if (digit >= 'a' && digit <= 'f')
        {
            return static_cast<std::uint8_t>((digit - 'a') + DECIMAL_DIGIT_COUNT);
        }

        throw std::invalid_argument("invalid hexadecimal digit");
    }

    std::vector<std::uint8_t> decode(const std::string& hex)
    {
        if (hex.size() % BYTE_HEX_WIDTH != 0)
        {
            throw std::invalid_argument("odd hexadecimal string length");
        }

        std::vector<std::uint8_t> bytes;
        bytes.reserve(hex.size() / BYTE_HEX_WIDTH);

        for (std::size_t i = 0; i < hex.size(); i += BYTE_HEX_WIDTH)
        {
            std::uint8_t high = digitToValue(hex[i]);
            std::uint8_t low = digitToValue(hex[i + 1]);
            std::uint8_t val = (high << NIBBLE_BITS) | low;

            bytes.push_back(val);
        }

        return bytes;
    }
}

namespace Tests
{
    using namespace HexConverter;

    void expectThrow(const std::string& hex)
    {
        bool caught = false;
        try
        {
            decode(hex);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught);
    }

    void run()
    {
        std::vector<std::uint8_t> bytes = {0x00, 0x0f, 0x10, 0xab, 0xff};

        assert(encode(bytes) == "000f10abff");
        assert(decode("000f10abff") == bytes);
        assert(decode(encode(bytes)) == bytes);
        assert(encode({}) == "");
        assert(decode("") == std::vector<std::uint8_t>{});
        assert(decode("7f") == std::vector<std::uint8_t>{0x7f});

        expectThrow("0");
        expectThrow("0g");
        expectThrow("0A");
    }
}

int main()
{
    Tests::run();
    return 0;
}