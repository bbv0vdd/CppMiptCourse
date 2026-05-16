#include <cmath>
#include <cctype>
#include <exception>
#include <istream>
#include <iterator>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace CalculatorV1
{
    class TokenStream
    {
    public:
        using Token = std::variant<char, double, std::string>;

        explicit TokenStream(const std::string& str) : m_stream(str + ';') {}

        bool empty()
        {
            m_stream >> std::ws;
            return m_stream.peek() == ';';
        }

        Token get()
        {
            if (m_hasToken)
            {
                m_hasToken = false;
                return m_token;
            }

            char ch = '\0';
            if (!(m_stream >> ch))
                return Token(';');

            switch (ch)
            {
                case '+': case '-': case '*': case '/': case '%': case '^': case '!':
                case '(': case ')': case '[': case ']': case '{': case '}': case ';':
                    return Token(ch);

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': case '.':
                {
                    m_stream.unget();
                    double val = 0.0;
                    m_stream >> val;
                    return Token(val);
                }

                default:
                {
                    if (!isNameStart(ch))
                        throw std::runtime_error("bad token");

                    std::string str(1, ch);
                    while (m_stream.get(ch) && isNamePart(ch))
                        str += ch;

                    if (!std::isspace(static_cast<unsigned char>(ch)))
                        m_stream.unget();

                    return Token(str);
                }
            }
        }

        void put(const Token& token)
        {
            m_token = token;
            m_hasToken = true;
        }

    private:
        static bool isNameStart(char ch)
        {
            return std::isalpha(static_cast<unsigned char>(ch)) != 0;
        }

        static bool isNamePart(char ch)
        {
            unsigned char uc = static_cast<unsigned char>(ch);
            return std::isalpha(uc) != 0 || std::isdigit(uc) != 0;
        }

        std::stringstream m_stream;
        Token m_token = Token(';');
        bool m_hasToken = false;
    };

    double factorial(double x)
    {
        if (x < 0.0 || std::trunc(x) != x)
            throw std::runtime_error("bad factorial");

        double result = 1.0;
        for (double f = 2.0; f <= x; f += 1.0)
            result *= f;

        return result;
    }

    double applyOp(char op, double left, double right)
    {
        switch (op)
        {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': return left / right;
            case '%': return std::fmod(left, right);
            case '^': return std::pow(left, right);
            default: throw std::runtime_error("bad operation");
        }
    }

    class Engine
    {
    public:
        double evaluate(const std::string& expr)
        {
            TokenStream stream(expr);

            if (stream.empty())
                throw std::runtime_error("empty expression");

            double val = parseStatement(stream);
            TokenStream::Token token = stream.get();

            if (!isChar(token, ';'))
                throw std::runtime_error("bad expression end");

            return val;
        }

    private:
        double parseStatement(TokenStream& stream)
        {
            TokenStream::Token token = stream.get();

            if (std::holds_alternative<std::string>(token))
            {
                if (std::get<std::string>(token) == "set")
                    return parseDeclaration(stream);
            }

            stream.put(token);
            return parseExpression(stream);
        }

        double parseDeclaration(TokenStream& stream)
        {
            TokenStream::Token token = stream.get();

            if (!std::holds_alternative<std::string>(token))
                throw std::runtime_error("name expected");

            std::string name = std::get<std::string>(token);
            double val = parseExpression(stream);
            m_vars[name] = val;

            return val;
        }

        double parseExpression(TokenStream& stream) const
        {
            double val = parseTerm(stream);
            TokenStream::Token token = stream.get();

            while (true)
            {
                if (!std::holds_alternative<char>(token))
                {
                    stream.put(token);
                    return val;
                }

                char op = std::get<char>(token);
                if (op == '+')
                    val += parseTerm(stream);
                else if (op == '-')
                    val -= parseTerm(stream);
                else
                {
                    stream.put(token);
                    return val;
                }

                token = stream.get();
            }
        }

        double parseTerm(TokenStream& stream) const
        {
            double val = parseSignedPower(stream);
            TokenStream::Token token = stream.get();

            while (true)
            {
                if (!std::holds_alternative<char>(token))
                {
                    stream.put(token);
                    return val;
                }

                char op = std::get<char>(token);
                if (op == '*')
                    val *= parseSignedPower(stream);
                else if (op == '/')
                    val /= parseSignedPower(stream);
                else if (op == '%')
                    val = std::fmod(val, parseSignedPower(stream));
                else
                {
                    stream.put(token);
                    return val;
                }

                token = stream.get();
            }
        }

        double parseSignedPower(TokenStream& stream) const
        {
            TokenStream::Token token = stream.get();

            if (std::holds_alternative<char>(token))
            {
                char op = std::get<char>(token);
                if (op == '+')
                    return parseSignedPower(stream);
                if (op == '-')
                    return -parseSignedPower(stream);
            }

            stream.put(token);
            return parsePower(stream);
        }

        double parsePower(TokenStream& stream) const
        {
            double val = parsePostfix(stream);
            TokenStream::Token token = stream.get();

            if (isChar(token, '^'))
                return std::pow(val, parseSignedPower(stream));

            stream.put(token);
            return val;
        }

        double parsePostfix(TokenStream& stream) const
        {
            double val = parsePrimary(stream);

            while (true)
            {
                TokenStream::Token token = stream.get();
                if (isChar(token, '!'))
                    val = factorial(val);
                else
                {
                    stream.put(token);
                    return val;
                }
            }
        }

        double parsePrimary(TokenStream& stream) const
        {
            TokenStream::Token token = stream.get();

            if (std::holds_alternative<char>(token))
            {
                char ch = std::get<char>(token);
                if (ch == '(') return parseGrouped(stream, ')');
                if (ch == '[') return parseGrouped(stream, ']');
                if (ch == '{') return parseGrouped(stream, '}');
            }

            if (std::holds_alternative<double>(token))
                return std::get<double>(token);

            if (std::holds_alternative<std::string>(token))
                return getVariable(std::get<std::string>(token));

            throw std::runtime_error("primary expected");
        }

        double parseGrouped(TokenStream& stream, char closing) const
        {
            double val = parseExpression(stream);
            TokenStream::Token token = stream.get();

            if (!isChar(token, closing))
                throw std::runtime_error("bad bracket");

            return val;
        }

        double getVariable(const std::string& name) const
        {
            auto it = m_vars.find(name);
            if (it == m_vars.end())
                throw std::runtime_error("unknown variable");

            return it->second;
        }

        static bool isChar(const TokenStream::Token& token, char val)
        {
            return std::holds_alternative<char>(token) && std::get<char>(token) == val;
        }

        std::unordered_map<std::string, double> m_vars;
    };

    bool near(double a, double b)
    {
        constexpr double TOL = 1e-9;
        return std::abs(a - b) <= TOL;
    }

    void requireClose(double a, double b)
    {
        if (!near(a, b))
            throw std::runtime_error("test failed");
    }

    void runTests()
    {
        Engine calc;

        requireClose(calc.evaluate("1 + 2 * 3"), 7.0);
        requireClose(calc.evaluate("46 % 6"), 4.0);
        requireClose(calc.evaluate("2 ^ 1 ^ 2"), 2.0);
        requireClose(calc.evaluate("6!"), 720.0);
        requireClose(calc.evaluate("[2 + 3] * {4 + 1}"), 25.0);
        requireClose(calc.evaluate("set a 6!"), 720.0);
        requireClose(calc.evaluate("a / (4!)"), 30.0);
        requireClose(calc.evaluate("-2 ^ 2"), -4.0);
    }

    void demo()
    {
        Engine calc;
        std::vector<std::string> examples = {
            "1 + 2 * 3",
            "46 % 6",
            "2 ^ 1 ^ 2",
            "6!",
            "[2 + 3] * {4 + 1}",
            "set a 6!",
            "a / (4!)"
        };

        std::print("CalculatorV1 demo:\n");
        for (const auto& ex : examples)
            std::print("{} = {}\n", ex, calc.evaluate(ex));
    }
}

namespace CalculatorV2
{
    namespace x3 = boost::spirit::x3;

    struct Sign;
    struct List;
    struct Factorial;

    class Operand : public x3::variant<double, x3::forward_ast<Sign>, x3::forward_ast<Factorial>, x3::forward_ast<List>>
    {
    public:
        using base_type::base_type;
        using base_type::operator=;
    };

    struct Sign
    {
        char op = '\0';
        Operand operand;
    };

    struct Step
    {
        char op = '\0';
        Operand operand;
    };

    struct List
    {
        Operand head;
        std::vector<Step> steps;
    };

    struct Factorial
    {
        Operand operand;
    };
}

BOOST_FUSION_ADAPT_STRUCT(CalculatorV2::Sign, op, operand)
BOOST_FUSION_ADAPT_STRUCT(CalculatorV2::Step, op, operand)
BOOST_FUSION_ADAPT_STRUCT(CalculatorV2::List, head, steps)

namespace CalculatorV2
{
    namespace parser
    {
        x3::rule<struct rule1, List> const expression = "expression";
        x3::rule<struct rule2, List> const term = "term";
        x3::rule<struct rule3, Operand> const signedPower = "signed_power";
        x3::rule<struct rule4, List> const power = "power";
        x3::rule<struct rule5, Operand> const postfix = "postfix";
        x3::rule<struct rule6, Operand> const primary = "primary";

        auto const assign = [](auto& ctx) { x3::_val(ctx) = x3::_attr(ctx); };
        auto const makeFactorial = [](auto& ctx) { x3::_val(ctx) = Factorial{x3::_val(ctx)}; };

        auto const expression_def = term >> *((x3::char_('+') >> term) | (x3::char_('-') >> term));
        auto const term_def = signedPower >> *((x3::char_('*') >> signedPower) | (x3::char_('/') >> signedPower) | (x3::char_('%') >> signedPower));
        auto const signedPower_def = (x3::char_('+') >> signedPower) | (x3::char_('-') >> signedPower) | power;
        auto const power_def = postfix >> *(x3::char_('^') >> signedPower);
        auto const postfix_def = primary[assign] >> *(x3::lit('!')[makeFactorial]);
        auto const primary_def = x3::double_ | ('(' >> expression >> ')') | ('[' >> expression >> ']') | ('{' >> expression >> '}');

        BOOST_SPIRIT_DEFINE(expression, term, signedPower, power, postfix, primary)
    }

    class Evaluator : public boost::static_visitor<double>
    {
    public:
        double operator()(double val) const { return val; }

        double operator()(const Sign& sign) const
        {
            double val = boost::apply_visitor(*this, sign.operand);
            return (sign.op == '+') ? val : -val;
        }

        double operator()(const Step& step, double left) const
        {
            double right = boost::apply_visitor(*this, step.operand);
            return CalculatorV1::applyOp(step.op, left, right);
        }

        double operator()(const List& list) const
        {
            double val = boost::apply_visitor(*this, list.head);
            for (const auto& step : list.steps)
                val = (*this)(step, val);
            return val;
        }

        double operator()(const Factorial& fact) const
        {
            return CalculatorV1::factorial(boost::apply_visitor(*this, fact.operand));
        }
    };

    double parse(std::string_view input)
    {
        auto begin = input.begin();
        auto end = input.end();

        List ast;
        bool ok = x3::phrase_parse(begin, end, parser::expression, x3::ascii::space, ast);

        if (!ok || begin != end)
            throw std::runtime_error("parse failed");

        Evaluator eval;
        return eval(ast);
    }

    void runTests()
    {
        CalculatorV1::requireClose(parse("1 + 2 * 3"), 7.0);
        CalculatorV1::requireClose(parse("46 % 6"), 4.0);
        CalculatorV1::requireClose(parse("2 ^ 1 ^ 2"), 2.0);
        CalculatorV1::requireClose(parse("6!"), 720.0);
        CalculatorV1::requireClose(parse("[2 + 3] * {4 + 1}"), 25.0);
        CalculatorV1::requireClose(parse("-2 ^ 3"), -8.0);
    }

    void demo()
    {
        std::vector<std::string> examples = {
            "1 + 2 * 3",
            "46 % 6",
            "2 ^ 1 ^ 2",
            "6!",
            "[2 + 3] * {4 + 1}",
            "-2 ^ 3"
        };

        std::print("CalculatorV2 demo:\n");
        for (const auto& ex : examples)
            std::print("{} = {}\n", ex, parse(ex));
    }
}

int main()
{
    try
    {
        CalculatorV1::runTests();
        CalculatorV2::runTests();

        std::print("tests passed\n\n");

        CalculatorV1::demo();
        std::print("\n");
        CalculatorV2::demo();
    }
    catch (const std::exception& e)
    {
        std::print("error: {}\n", e.what());
        return 1;
    }

    return 0;
}