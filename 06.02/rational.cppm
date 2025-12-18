export module rational;

import <compare>;

export namespace my_rational
{
    class Rational
    {
    public:
        Rational(int num = 0, int den = 1);

        explicit operator double() const
        {
            return 1.0 * m_num / m_den;
        }

        Rational & operator+=(Rational const & other);
        Rational & operator-=(Rational const & other);
        Rational & operator*=(Rational const & other);
        Rational & operator/=(Rational const & other);

        const Rational operator++(int);
        const Rational operator--(int);

        Rational & operator++();
        Rational & operator--();

        friend Rational operator+(Rational lhs, Rational const & rhs);
        friend Rational operator-(Rational lhs, Rational const & rhs);
        friend Rational operator*(Rational lhs, Rational const & rhs);
        friend Rational operator/(Rational lhs, Rational const & rhs);

        friend std::strong_ordering operator<=>(Rational const & lhs, Rational const & rhs);
        friend bool operator==(Rational const & lhs, Rational const & rhs);

    private:
        void reduce();
        int m_num;
        int m_den;
    };
}