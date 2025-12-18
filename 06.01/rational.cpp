#include "rational.hpp" 
#include <numeric>   

Rational::Rational(int num, int den) : m_num(num), m_den(den)
{
    reduce();
}

Rational & Rational::operator+=(Rational const & other)
{
    auto lcm = std::lcm(m_den, other.m_den);

    m_num = m_num * (lcm / m_den) + other.m_num * (lcm / other.m_den);
    
    m_den = lcm;

    reduce();

    return *this;
}


Rational & Rational::operator*=(Rational const & other)
{
    m_num *= other.m_num;
    m_den *= other.m_den;

    reduce();

    return *this;
}


void Rational::reduce()
{
    if (m_den < 0)
    {
        m_num = -m_num;
        m_den = -m_den;
    }

    auto gcd = std::gcd(m_num, m_den);

    m_num /= gcd;
    m_den /= gcd;
}


Rational operator+(Rational lhs, Rational const & rhs)
{
    return lhs += rhs;
}

Rational operator-(Rational lhs, Rational const & rhs)
{
    return lhs -= rhs;
}

Rational operator*(Rational lhs, Rational const & rhs)
{
    return lhs *= rhs;
}

Rational operator/(Rational lhs, Rational const & rhs)
{
    return lhs /= rhs;
}

std::strong_ordering operator<=>(Rational const & lhs, Rational const & rhs)
{
    int64_t const left  = static_cast<int64_t>(lhs.m_num) * rhs.m_den;
    int64_t const right = static_cast<int64_t>(rhs.m_num) * lhs.m_den;

    if (left < right)
    {
        return std::strong_ordering::less;
    }
    if (left > right)
    {
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}

bool operator==(Rational const & lhs, Rational const & rhs)
{
    return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
}

std::istream & operator>>(std::istream & stream, Rational & rational)
{
    stream >> rational.m_num;
    if (stream.peek() == '/')
    {
        stream.ignore();
        stream >> rational.m_den;
    }
    else
    {
        rational.m_den = 1;
    }
    rational.reduce();
    return stream;
}

std::ostream & operator<<(std::ostream & stream, Rational const & rational)
{
    return stream << rational.m_num << '/' << rational.m_den;
}

void linkingErrorMultipleDefinedReference() {}