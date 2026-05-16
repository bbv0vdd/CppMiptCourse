#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace CommentRemover
{
    bool isSpace(char ch)
    {
        return std::isspace(static_cast<unsigned char>(ch)) != 0;
    }

    bool isEscaped(const std::string& text, std::string::size_type pos)
    {
        std::size_t backslashes = 0;
        while (pos > 0 && text[pos - 1] == '\\')
        {
            ++backslashes;
            --pos;
        }
        return backslashes % 2 != 0;
    }

    std::string::size_type skipQuoted(const std::string& text, std::string::size_type pos, char quote)
    {
        ++pos;
        while (pos < text.size())
        {
            if (text[pos] == quote && !isEscaped(text, pos))
                return pos + 1;
            ++pos;
        }
        return text.size();
    }

    std::string::size_type rawQuotePos(const std::string& text, std::string::size_type pos)
    {
        if (pos + 1 < text.size() && text[pos] == 'R' && text[pos + 1] == '"')
            return pos + 1;

        if (pos + 2 < text.size() && (text[pos] == 'u' || text[pos] == 'U' || text[pos] == 'L') && text[pos + 1] == 'R' && text[pos + 2] == '"')
            return pos + 2;

        if (pos + 3 < text.size() && text[pos] == 'u' && text[pos + 1] == '8' && text[pos + 2] == 'R' && text[pos + 3] == '"')
            return pos + 3;

        return std::string::npos;
    }

    bool isValidDelimiterChar(char ch)
    {
        return ch != ' ' && ch != '(' && ch != ')' && ch != '\\' &&
               ch != '\t' && ch != '\n' && ch != '\r' && ch != '\v' && ch != '\f';
    }

    std::string::size_type skipRawString(const std::string& text, std::string::size_type pos)
    {
        std::size_t quotePos = rawQuotePos(text, pos);
        if (quotePos == std::string::npos)
            return pos;

        std::size_t delimStart = quotePos + 1;
        std::size_t openParen = delimStart;

        while (openParen < text.size() && text[openParen] != '(')
        {
            if (!isValidDelimiterChar(text[openParen]))
                return pos;
            ++openParen;
        }

        if (openParen == text.size())
            return pos;

        std::string delim = text.substr(delimStart, openParen - delimStart);
        std::string terminator = ")" + delim + '"';
        std::size_t termPos = text.find(terminator, openParen + 1);

        if (termPos == std::string::npos)
            return text.size();

        return termPos + terminator.size();
    }

    void stripComments(std::string& code)
    {
        std::size_t i = 0;
        while (i < code.size())
        {
            std::size_t rawEnd = skipRawString(code, i);
            if (rawEnd != i)
            {
                i = rawEnd;
                continue;
            }

            if (code[i] == '"')
            {
                i = skipQuoted(code, i, '"');
                continue;
            }

            if (code[i] == '\'')
            {
                i = skipQuoted(code, i, '\'');
                continue;
            }

            if (i + 1 < code.size() && code[i] == '/' && code[i + 1] == '/')
            {
                std::size_t end = code.find('\n', i + 2);
                if (end == std::string::npos)
                    code.erase(i);
                else
                    code.erase(i, end - i);
                continue;
            }

            if (i + 1 < code.size() && code[i] == '/' && code[i + 1] == '*')
            {
                std::size_t end = code.find("*/", i + 2);
                if (end == std::string::npos)
                    code.erase(i);
                else
                    code.erase(i, end + 2 - i);
                continue;
            }

            ++i;
        }
    }

    bool isBlankLine(const std::string& line)
    {
        for (char ch : line)
            if (!isSpace(ch))
                return false;
        return true;
    }

    std::string removeEmptyLines(const std::string& text)
    {
        std::string result;
        std::size_t lineStart = 0;

        while (lineStart < text.size())
        {
            std::size_t lineEnd = text.find('\n', lineStart);
            bool hasNewline = lineEnd != std::string::npos;

            if (!hasNewline)
                lineEnd = text.size();

            std::string line = text.substr(lineStart, lineEnd - lineStart);

            if (!isBlankLine(line))
            {
                result += line;
                if (hasNewline)
                    result += '\n';
            }

            if (!hasNewline)
                break;

            lineStart = lineEnd + 1;
        }

        return result;
    }

    class Cleaner
    {
    public:
        void processFile(const std::string& inputPath, const std::string& outputPath) const
        {
            std::string content = readFile(inputPath);
            stripComments(content);
            content = removeEmptyLines(content);
            writeFile(outputPath, content);
        }

        std::string processText(std::string text) const
        {
            stripComments(text);
            return removeEmptyLines(text);
        }

    private:
        static std::string readFile(const std::string& path)
        {
            std::ifstream file(path, std::ios::in);
            if (!file)
                throw std::runtime_error("input file error");

            return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }

        static void writeFile(const std::string& path, const std::string& content)
        {
            std::ofstream file(path, std::ios::out);
            if (!file)
                throw std::runtime_error("output file error");

            file << content;
        }
    };
}

namespace Tests
{
    using namespace CommentRemover;

    void testSimple()
    {
        const std::string source =
            "int main()\n"
            "{\n"
            "    auto a = 1; // line comment\n"
            "\n"
            "    auto text = \"not // comment\";\n"
            "    auto ch = '/';\n"
            "    auto raw = R\"tag(raw // text\n"
            "raw /* text */\n"
            ")tag\";\n"
            "    /* block\n"
            "       comment */\n"
            "    auto b = 2;\n"
            "       \t  \n"
            "}\n";

        const std::string expected =
            "int main()\n"
            "{\n"
            "    auto a = 1; \n"
            "    auto text = \"not // comment\";\n"
            "    auto ch = '/';\n"
            "    auto raw = R\"tag(raw // text\n"
            "raw /* text */\n"
            ")tag\";\n"
            "    auto b = 2;\n"
            "}\n";

        Cleaner cleaner;
        assert(cleaner.processText(source) == expected);
    }

    void testRawPrefix()
    {
        const std::string source =
            "auto a = u8R\"x(// keep\n"
            "/* keep */)x\";\n"
            "auto b = LR\"x(// keep)x\"; // remove\n"
            "auto c = UR\"x(/* keep */)x\";\n";

        const std::string expected =
            "auto a = u8R\"x(// keep\n"
            "/* keep */)x\";\n"
            "auto b = LR\"x(// keep)x\"; \n"
            "auto c = UR\"x(/* keep */)x\";\n";

        Cleaner cleaner;
        assert(cleaner.processText(source) == expected);
    }

    void testFile()
    {
        const std::string inputPath = "source.cpp";
        const std::string outputPath = "output.cpp";

        {
            std::ofstream out(inputPath);
            if (!out)
                throw std::runtime_error("test file error");

            out << "auto value = 10; // remove\n"
                << "\n"
                << "auto text = R\"(// keep)\";\n";
        }

        Cleaner cleaner;
        cleaner.processFile(inputPath, outputPath);

        std::ifstream in(outputPath);
        if (!in)
            throw std::runtime_error("result file error");

        std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

        const std::string expected =
            "auto value = 10; \n"
            "auto text = R\"(// keep)\";\n";

        assert(result == expected);

        std::filesystem::remove(inputPath);
        std::filesystem::remove(outputPath);
    }

    void runAll()
    {
        testSimple();
        testRawPrefix();
        testFile();
    }
}

int main()
{
    Tests::runAll();
    std::cout << "All tests passed\n";
    return 0;
}