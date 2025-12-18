module rational;

import <numeric>;
import <compare>;
import <cstdint>;

namespace my_rational
{
    void Rational::reduce()
    {
        if (m_den < 0)
        {
            m_num = -m_num;
            m_den = -m_den;
        }

        auto const common_divisor = std::gcd(m_num, m_den);
        m_num /= common_divisor;
        m_den /= common_divisor;
    }

    Rational::Rational(int num, int den) : m_num(num), m_den(den)
    {
        reduce();
    }

    Rational & Rational::operator+=(Rational const & other)
    {
        auto const common_denominator = std::lcm(m_den, other.m_den);
        m_num = m_num * (common_denominator / m_den) + other.m_num * (common_denominator / other.m_den);
        m_den = common_denominator;
        reduce();
        return *this;
    }

    Rational & Rational::operator-=(Rational const & other)
    {
        return *this += Rational(other.m_num * -1, other.m_den);
    }

    Rational & Rational::operator*=(Rational const & other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational & Rational::operator/=(Rational const & other)
    {
        return *this *= Rational(other.m_den, other.m_num);
    }

    const Rational Rational::operator++(int)
    {
        auto temp = *this;
        *this += 1;
        return temp;
    }

    const Rational Rational::operator--(int)
    {
        auto temp = *this;
        *this -= 1;
        return temp;
    }

    Rational & Rational::operator++()
    {
        *this += 1;
        return *this;
    }

    Rational & Rational::operator--()
    {
        *this -= 1;
        return *this;
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
        int64_t const left = static_cast<int64_t>(lhs.m_num) * rhs.m_den;
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
}