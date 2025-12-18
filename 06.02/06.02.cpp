/*
./build.sh
./build/06.02.out
*/

#include <cassert>
import <cmath>;
import <sstream>;
import <iostream>;

import rational;

using namespace my_rational;

bool equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

int main()
{
    Rational x = 1, y(2, 1);

    //  -----------------------------------------------------------------------

    assert(equal(static_cast < double > (x), 1));

    //  -----------------------------------------------------------------------

    assert((x += y) == Rational(+3, 1));
    assert((x -= y) == Rational(+1, 1));
    assert((x *= y) == Rational(+2, 1));
    assert((x /= y) == Rational(+1, 1));

    //  -----------------------------------------------------------------------

    assert((x++) == Rational(+1, 1));
    assert(x == Rational(2, 1));
    assert((x--) == Rational(+2, 1));
    assert(x == Rational(1, 1));
    assert((++y) == Rational(+3, 1));
    assert((--y) == Rational(+2, 1));

    //  -----------------------------------------------------------------------

    assert((x +  y) == Rational(+3, 1));
    assert((x -  y) == Rational(-1, 1));
    assert((x *  y) == Rational(+2, 1));
    assert((x /  y) == Rational(+1, 2));

    //  -----------------------------------------------------------------------

    assert((x += 1) == Rational(+2, 1));
    assert((x +  1) == Rational(+3, 1));
    assert((1 +  y) == Rational(+3, 1));
    assert((Rational(1) + 1) == Rational(+2, 1));

    //  -----------------------------------------------------------------------
    
    x = Rational(1, 1);
    y = Rational(2, 1);

    assert((x <  y) == true);
    assert((x >  y) == false);
    assert((x <= y) == true);
    assert((x >= y) == false);
    assert((x == y) == false);
    assert((x != y) == true);
    
    y = Rational(1, 1);
    assert((x == y) == true);

}
