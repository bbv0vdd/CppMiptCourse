#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace QuadraticEquation
{
    constexpr double EPSILON = 1.0e-7;

    using RootType = std::variant<double, std::pair<double, double>, std::monostate>;
    using Result = std::optional<RootType>;

    class RootPrinter
    {
    public:
        explicit RootPrinter(std::ostream& output)
            : out(output)
        {
        }

        void operator()(double root) const
        {
            out << "One root: " << root << '\n';
        }

        void operator()(std::pair<double, double> const& roots) const
        {
            out << "Two roots: " << roots.first << ' ' << roots.second << '\n';
        }

        void operator()(std::monostate) const
        {
            out << "Infinite number of roots.\n";
        }

    private:
        std::ostream& out;
    };

    bool isZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    double calculateDiscriminant(double a, double b, double c)
    {
        return b * b - 4.0 * a * c;
    }

    double calculateRoot(double b, double a)
    {
        return -b / (2.0 * a);
    }

    std::pair<double, double> calculateRoots(double b, double a, double discriminant)
    {
        double sqrtDisc = std::sqrt(discriminant);
        double denominator = 2.0 * a;
        return {(-b + sqrtDisc) / denominator, (-b - sqrtDisc) / denominator};
    }

    Result solve(double a, double b, double c)
    {
        if (isZero(a))
        {
            if (isZero(b) && isZero(c))
            {
                return std::monostate{};
            }
            throw std::invalid_argument("Error: Not a quadratic equation.");
        }

        double discriminant = calculateDiscriminant(a, b, c);

        if (discriminant < -EPSILON)
        {
            return std::nullopt;
        }

        if (isZero(discriminant))
        {
            return calculateRoot(b, a);
        }

        return calculateRoots(b, a, discriminant);
    }

    std::string toString(Result const& result)
    {
        std::ostringstream output;
        
        if (!result.has_value())
        {
            output << "No real roots.\n";
        }
        else
        {
            std::visit(RootPrinter(output), result.value());
        }
        
        return output.str();
    }
}

namespace Tests
{
    using namespace QuadraticEquation;

    void testInfiniteRoots()
    {
        assert(toString(solve(0.0, 0.0, 0.0)) == "Infinite number of roots.\n");
    }

    void testImpossibleEquation()
    {
        bool caught = false;
        try
        {
            solve(0.0, 0.0, 5.0);
        }
        catch (std::invalid_argument const&)
        {
            caught = true;
        }
        assert(caught);
    }

    void testLinearEquation()
    {
        bool caught = false;
        try
        {
            solve(0.0, 2.0, -4.0);
        }
        catch (std::invalid_argument const&)
        {
            caught = true;
        }
        assert(caught);
    }

    void testNoRealRoots()
    {
        assert(toString(solve(1.0, 0.0, 1.0)) == "No real roots.\n");
    }

    void testOneRoot()
    {
        assert(toString(solve(1.0, -2.0, 1.0)) == "One root: 1\n");
    }

    void testTwoRoots()
    {
        assert(toString(solve(1.0, 0.0, -4.0)) == "Two roots: 2 -2\n");
    }

    void runAll()
    {
        testInfiniteRoots();
        testImpossibleEquation();
        testLinearEquation();
        testNoRealRoots();
        testOneRoot();
        testTwoRoots();
    }
}

int main()
{
    Tests::runAll();
    return 0;
}