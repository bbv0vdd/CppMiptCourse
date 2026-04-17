/*
g++ -std=c++23 -Wall -Wextra -Wpedantic -O3 -m32 10.05.cpp -o 10.05.out
./10.05.out
*/

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

using Digest = std::uint32_t;
using Transform = std::function<Digest(std::string const &)>;

namespace 
{
    constexpr std::size_t TOTAL_TRANSFORMS = 9;
    constexpr std::size_t POOL_SIZE = 20000;
    constexpr std::size_t INCREMENT = 250;
    constexpr std::size_t EXPERIMENT_REPEATS = 8;
    constexpr std::size_t SHORTEST = 4;
    constexpr std::size_t LONGEST = 40;
    constexpr std::uint64_t MAGIC_START = 0xC0FFEE1234567890ULL;

    struct TransformRecord
    {
        std::string m_label;
        Transform m_method;
    };

    struct ExperimentSettings // why not namespace? if these are just constants why passing them around as const references, why not use them as global variables in namespace?
    {
        std::size_t m_poolSize = POOL_SIZE;
        std::size_t m_step = INCREMENT;
        std::size_t m_repeats = EXPERIMENT_REPEATS;
        std::size_t m_minLen = SHORTEST;
        std::size_t m_maxLen = LONGEST;
        std::string m_outputPath = "hash_collisions.csv";
    };

    class TextGenerator
    {
    public:
        explicit TextGenerator(std::uint64_t seed)
            : m_generator(seed),
              m_lenDist(static_cast<int>(SHORTEST), static_cast<int>(LONGEST))
        {
        }

        [[nodiscard]] std::string generate() // why are you using own generator: https://github.com/i-s-m-mipt/Education/blob/master/projects/examples/source/10.42.cpp 
        {
            static constexpr char symbols[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

            std::string buffer;
            int length = m_lenDist(m_generator);
            buffer.reserve(static_cast<std::size_t>(length));

            std::uniform_int_distribution<int> symDist(0, static_cast<int>(sizeof(symbols) - 2));

            for (int i = 0; i < length; ++i)
            {
                buffer.push_back(symbols[symDist(m_generator)]);
            }

            return buffer;
        }

    private:
        std::mt19937_64 m_generator;
        std::uniform_int_distribution<int> m_lenDist;
    };

    // Robert Sedgwicks
    [[nodiscard]] Digest rs_hash(std::string const & input)
    {
        std::uint32_t b = 378551U;
        std::uint32_t a = 63689U;
        std::uint32_t h = 0U;

        for (unsigned char c : input)
        {
            h = h * a + c;
            a *= b;
        }

        return h;
    }

    // Justin Sobel
    [[nodiscard]] Digest js_hash(std::string const & input)
    {
        std::uint32_t h = 1315423911U;

        for (unsigned char c : input)
        {
            h ^= (h << 5) + c + (h >> 2);
        }

        return h;
    }

    // Peter J. Weinberger
    [[nodiscard]] Digest pjw_hash(std::string const & input)
    {
        constexpr std::uint32_t BITS = 32U;
        constexpr std::uint32_t THREE_Q = (BITS * 3U) / 4U;
        constexpr std::uint32_t EIGHTH = BITS / 8U;
        constexpr std::uint32_t HIGH_BITS = 0xFFFFFFFFU << (BITS - EIGHTH);

        std::uint32_t h = 0U;
        std::uint32_t test = 0U;

        for (unsigned char c : input)
        {
            h = (h << EIGHTH) + c;
            test = h & HIGH_BITS;

            if (test != 0U)
            {
                h = (h ^ (test >> THREE_Q)) & (~HIGH_BITS);
            }
        }

        return h;
    }

    // Executable and Linkable Format
    [[nodiscard]] Digest elf_hash(std::string const & input)
    {
        std::uint32_t h = 0U;
        std::uint32_t x = 0U;

        for (unsigned char c : input)
        {
            h = (h << 4) + c;
            x = h & 0xF0000000U;

            if (x != 0U)
            {
                h ^= x >> 24U;
            }

            h &= ~x;
        }

        return h;
    }

    // Brian Kernighan & Dennis Ritchie
    [[nodiscard]] Digest bkdr_hash(std::string const & input)
    {
        constexpr std::uint32_t BASE = 131U;
        std::uint32_t h = 0U;

        for (unsigned char c : input)
        {
            h = h * BASE + c;
        }

        return h;
    }

    // SDBM (public domain hash)
    [[nodiscard]] Digest sdbm_hash(std::string const & input)
    {
        std::uint32_t h = 0U;

        for (unsigned char c : input)
        {
            h = c + (h << 6U) + (h << 16U) - h;
        }

        return h;
    }

    // Daniel J. Bernstein
    [[nodiscard]] Digest djb_hash(std::string const & input)
    {
        std::uint32_t h = 5381U;

        for (unsigned char c : input)
        {
            h = ((h << 5U) + h) + c;
        }

        return h;
    }

    // Donald E. Knuth
    [[nodiscard]] Digest dek_hash(std::string const & input)
    {
        std::uint32_t h = static_cast<std::uint32_t>(input.length());

        for (unsigned char c : input)
        {
            h = ((h << 5U) ^ (h >> 27U)) ^ c;
        }

        return h;
    }

    // Bitwise shift (simple)
    [[nodiscard]] Digest bp_hash(std::string const & input)
    {
        std::uint32_t h = 0U;

        for (unsigned char c : input)
        {
            h = (h << 7U) ^ c;
        }

        return h;
    }

    [[nodiscard]] std::vector<TransformRecord> buildTransformSet()
    {
        std::vector<TransformRecord> transforms;
        transforms.reserve(TOTAL_TRANSFORMS);

        transforms.push_back({"RS", rs_hash});
        transforms.push_back({"JS", js_hash});
        transforms.push_back({"PJW", pjw_hash});
        transforms.push_back({"ELF", elf_hash});
        transforms.push_back({"BKDR", bkdr_hash});
        transforms.push_back({"SDBM", sdbm_hash});
        transforms.push_back({"DJB", djb_hash});
        transforms.push_back({"DEK", dek_hash});
        transforms.push_back({"BP", bp_hash});

        return transforms;
    }

    [[nodiscard]] std::vector<std::string> buildTextPool(
        std::size_t count,
        std::uint64_t seed)
    {
        TextGenerator fabric(seed);

        std::vector<std::string> pool;
        pool.reserve(count);

        for (std::size_t i = 0; i < count; ++i)
        {
            pool.push_back(fabric.generate());
        }

        return pool;
    }

    [[nodiscard]] std::size_t measureConflicts(
        std::vector<std::string> const & pool,
        std::size_t limit,
        Transform const & method)
    {
        std::unordered_set<Digest> seen;
        seen.reserve(limit * 2U);

        for (std::size_t i = 0; i < limit; ++i)
        {
            seen.insert(method(pool[i]));
        }

        return limit - seen.size();
    }

    void emitHeader(std::ofstream & out)
    {
        out
            << "hash_name,"
            << "string_count,"
            << "series_index,"
            << "collision_count\n";
    }

    void exportData(
        ExperimentSettings const & cfg,
        std::vector<TransformRecord> const & transforms)
    {
        std::ofstream out(cfg.m_outputPath);

        if (!out)
        {
            throw std::runtime_error("cannot open output file");
        }

        emitHeader(out);

        for (std::size_t cycle = 0; cycle < cfg.m_repeats; ++cycle)
        {
            const std::uint64_t currentSeed =
                MAGIC_START + static_cast<std::uint64_t>(cycle);

            const std::vector<std::string> textPool =
                buildTextPool(cfg.m_poolSize, currentSeed);

            for (TransformRecord const & record : transforms)
            {
                for (std::size_t cnt = cfg.m_step;
                     cnt <= cfg.m_poolSize;
                     cnt += cfg.m_step)
                {
                    const std::size_t conflicts =
                        measureConflicts(textPool, cnt, record.m_method);

                    out
                        << record.m_label << ','
                        << cnt << ','
                        << cycle << ','
                        << conflicts << '\n';
                }
            }
        }
    }

    void validateTransforms(std::vector<TransformRecord> const & transforms)
    {
        assert(transforms.size() == TOTAL_TRANSFORMS);

        for (TransformRecord const & record : transforms)
        {
            const Digest first = record.m_method("abc123");
            const Digest second = record.m_method("abc123");
            const Digest third = record.m_method("abc124");

            assert(first == second);

            if (record.m_label != "BP")
            {
                assert(first != third);
            }
        }

        {
            std::vector<std::string> sample = {"aa", "bb", "cc"};
            const std::size_t conflicts =
                measureConflicts(sample, sample.size(), transforms[0].m_method);

            assert(conflicts <= sample.size());
        }

        {
            const std::vector<std::string> sample =
                buildTextPool(128U, MAGIC_START);

            assert(sample.size() == 128U);

            for (std::string const & item : sample)
            {
                assert(item.size() >= SHORTEST);
                assert(item.size() <= LONGEST);
            }
        }
    }

    void demonstration(std::vector<TransformRecord> const & transforms)
    {
        static constexpr std::string_view demoText =
            "abcdefghijklmnopqrstuvwxyz1234567890";

        std::cout << "Demo text: " << demoText << '\n';

        for (TransformRecord const & record : transforms)
        {
            std::cout
                << std::setw(4) << record.m_label
                << " -> "
                << record.m_method(std::string(demoText))
                << '\n';
        }

        std::cout << '\n';
    }
}

int main()
{
    try
    {
        const std::vector<TransformRecord> transforms = buildTransformSet();

        validateTransforms(transforms);
        demonstration(transforms);

        ExperimentSettings params;
        exportData(params, transforms);

        std::cout
            << "CSV saved to: " << params.m_outputPath << '\n'
            << "Rows per transform: "
            << (params.m_poolSize / params.m_step) * params.m_repeats
            << '\n'
            << "Transform count: " << transforms.size() << '\n';

        return 0;
    }
    catch (std::exception const & ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Error: unknown exception\n";
    }

    return 1;
}


/*
 * Score is 4/10
 * 1. your graph is very coarse, you need more points on the x-axis and show some kind of monotonic increase in the number of collisions
 * 2. just use generator from template https://github.com/i-s-m-mipt/Education/blob/master/projects/examples/source/10.42.cpp
 * 3. need justification, discussion and conclusion of the plot 
 * 10.05 "Обоснуйте форму полученных зависимостей. Определите лучшие и худшие хэш-функции данного набора"
 */
