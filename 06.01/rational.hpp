#pragma once 

#include <istream>
#include <ostream>
#include <compare>
#include <cstdint>

class Rational
{
public:

    Rational(int num = 0, int den = 1);

    explicit operator double() const
    {
        return 1.0 * m_num / m_den;
    }

    Rational & operator+=(Rational const & other);
    Rational & operator*=(Rational const & other);

    Rational & operator-=(Rational const & other)
    {
        return *this += Rational(other.m_num * -1, other.m_den);
    }

    Rational & operator/=(Rational const & other)
    {
        return *this *= Rational(other.m_den, other.m_num);
    }

    const Rational operator++(int)
    {
        auto x = *this;
        *this += 1;
        return x;
    }

    const Rational operator--(int)
    {
        auto x = *this;
        *this -= 1;
        return x;
    }

    Rational & operator++()
    {
        *this += 1;
        return *this;
    }

    Rational & operator--()
    {
        *this -= 1;
        return *this;
    }

    friend Rational operator+(Rational lhs, Rational const & rhs);
    friend Rational operator-(Rational lhs, Rational const & rhs);
    friend Rational operator*(Rational lhs, Rational const & rhs);
    friend Rational operator/(Rational lhs, Rational const & rhs);

    friend std::strong_ordering operator<=>(Rational const & lhs, Rational const & rhs);
    friend bool operator==(Rational const & lhs, Rational const & rhs);

    friend std::istream & operator>>(std::istream & stream, Rational & rational);
    friend std::ostream & operator<<(std::ostream & stream, Rational const & rational);

private:
    
	void reduce();

    int m_num = 0, m_den = 1;
};

void linkingErrorUndefinedReference();

// ERROR: redefinition
// void linkingErrorMultipleDefinedReference() {} 
