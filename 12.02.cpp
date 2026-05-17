#include <cstddef>
#include <cstdio>

namespace Quine
{
    class Printer
    {
    public:
        explicit Printer(const char* source, const char* marker)
            : m_source(source)
            , m_marker(marker)
        {
        }

        void print() const
        {
            bool markerUsed = false;
            const auto markerLen = length(m_marker);

            for (const char* pos = m_source; *pos != '\0';)
            {
                if (!markerUsed && startsWith(pos, m_marker))
                {
                    printSourceLiteral(m_source);
                    pos += markerLen;
                    markerUsed = true;
                }
                else
                {
                    std::printf("%c", *pos);
                    ++pos;
                }
            }
        }

    private:
        static std::size_t length(const char* text)
        {
            std::size_t len = 0;
            while (text[len] != '\0')
            {
                ++len;
            }
            return len;
        }

        static bool startsWith(const char* text, const char* prefix)
        {
            for (std::size_t i = 0; prefix[i] != '\0'; ++i)
            {
                if (text[i] != prefix[i])
                {
                    return false;
                }
            }
            return true;
        }

        static void printEscapedChar(char ch)
        {
            const char newline = '\n';
            const char quote = '"';
            const char backslash = '\\';

            if (ch == newline)
            {
                std::printf("\\n");
            }
            else if (ch == quote)
            {
                std::printf("\\\"");
            }
            else if (ch == backslash)
            {
                std::printf("\\\\");
            }
            else
            {
                std::printf("%c", ch);
            }
        }

        static void printSourceLiteral(const char* source)
        {
            const char newline = '\n';
            const char quote = '"';
            const char* indent = "        ";

            std::printf("%s%c", indent, quote);

            for (const char* pos = source; *pos != '\0'; ++pos)
            {
                printEscapedChar(*pos);

                if (*pos == newline)
                {
                    std::printf("%c%c%s%c", quote, newline, indent, quote);
                }
            }

            std::printf("%c;%c", quote, newline);
        }

        const char* m_source;
        const char* m_marker;
    };
}

namespace Code
{
    const char* const SOURCE =
        "/*\n"
        "g++ -std=c++23 -Wall -Wextra -Wpedantic 12.02.cpp -o 12.02.out\n"
        "./12.02.out\n"
        "./12.02.out > 12.02.generated.cpp\n"
        "diff -u 12.02.cpp 12.02.generated.cpp\n"
        "*/\n"
        "\n"
        "#include <cstddef>\n"
        "#include <cstdio>\n"
        "\n"
        "namespace Quine\n"
        "{\n"
        "    class Printer\n"
        "    {\n"
        "    public:\n"
        "        explicit Printer(const char* source, const char* marker)\n"
        "            : m_source(source)\n"
        "            , m_marker(marker)\n"
        "        {\n"
        "        }\n"
        "\n"
        "        void print() const\n"
        "        {\n"
        "            bool markerUsed = false;\n"
        "            const auto markerLen = length(m_marker);\n"
        "\n"
        "            for (const char* pos = m_source; *pos != '\\0';)\n"
        "            {\n"
        "                if (!markerUsed && startsWith(pos, m_marker))\n"
        "                {\n"
        "                    printSourceLiteral(m_source);\n"
        "                    pos += markerLen;\n"
        "                    markerUsed = true;\n"
        "                }\n"
        "                else\n"
        "                {\n"
        "                    std::printf(\"%c\", *pos);\n"
        "                    ++pos;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "\n"
        "    private:\n"
        "        static std::size_t length(const char* text)\n"
        "        {\n"
        "            std::size_t len = 0;\n"
        "            while (text[len] != '\\0')\n"
        "            {\n"
        "                ++len;\n"
        "            }\n"
        "            return len;\n"
        "        }\n"
        "\n"
        "        static bool startsWith(const char* text, const char* prefix)\n"
        "        {\n"
        "            for (std::size_t i = 0; prefix[i] != '\\0'; ++i)\n"
        "            {\n"
        "                if (text[i] != prefix[i])\n"
        "                {\n"
        "                    return false;\n"
        "                }\n"
        "            }\n"
        "            return true;\n"
        "        }\n"
        "\n"
        "        static void printEscapedChar(char ch)\n"
        "        {\n"
        "            const char newline = '\\n';\n"
        "            const char quote = '\"';\n"
        "            const char backslash = '\\\\';\n"
        "\n"
        "            if (ch == newline)\n"
        "            {\n"
        "                std::printf(\"\\\\n\");\n"
        "            }\n"
        "            else if (ch == quote)\n"
        "            {\n"
        "                std::printf(\"\\\\\\\"\");\n"
        "            }\n"
        "            else if (ch == backslash)\n"
        "            {\n"
        "                std::printf(\"\\\\\\\\\");\n"
        "            }\n"
        "            else\n"
        "            {\n"
        "                std::printf(\"%c\", ch);\n"
        "            }\n"
        "        }\n"
        "\n"
        "        static void printSourceLiteral(const char* source)\n"
        "        {\n"
        "            const char newline = '\\n';\n"
        "            const char quote = '\"';\n"
        "            const char* indent = \"        \";\n"
        "\n"
        "            std::printf(\"%s%c\", indent, quote);\n"
        "\n"
        "            for (const char* pos = source; *pos != '\\0'; ++pos)\n"
        "            {\n"
        "                printEscapedChar(*pos);\n"
        "\n"
        "                if (*pos == newline)\n"
        "                {\n"
        "                    std::printf(\"%c%c%s%c\", quote, newline, indent, quote);\n"
        "                }\n"
        "            }\n"
        "\n"
        "            std::printf(\"%c;%c\", quote, newline);\n"
        "        }\n"
        "\n"
        "        const char* m_source;\n"
        "        const char* m_marker;\n"
        "    };\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    const char* const marker = \"@@SOURCE@@\";\n"
        "    Quine::Printer printer(Code::SOURCE, marker);\n"
        "    printer.print();\n"
        "}\n";

    const char* const MARKER = "@@SOURCE@@";
}

int main()
{
    Quine::Printer printer(Code::SOURCE, Code::MARKER);
    printer.print();
}

// Source code and output do not match 
// "g++ -std=c++23 -Wall -Wextra -Wpedantic 12.02.cpp -o 12.02.out\n" is not in the code
