/*
./build.sh
./06.01.out
*/

#include "rational.hpp"

#include <cassert>
#include <cmath>
#include <sstream>

auto equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

int main()
{
	// ERROR: undefined reference
	// linkingErrorUndefinedReference(); 

	// ERROR: function was not declared
	// linkingErrorUndefinedFunction(); 

	// ERROR: redefinition
	// нужно расскомментировать объявление функции linkingErrorMultipleDefinedReference на 77 строке rational.hpp

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
    assert((1 +  1) == Rational(+2, 1)); 

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


    //  -----------------------------------------------------------------------

    std::stringstream stream_1("1/2");
    std::stringstream stream_2;
    
    //  -----------------------------------------------------------------------

    stream_1 >> x;
    stream_2 << x;

    //  -----------------------------------------------------------------------

    assert(stream_2.str() == "1/2");
}
