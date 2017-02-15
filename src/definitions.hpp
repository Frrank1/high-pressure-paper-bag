/**
 *  Simple types and function definitions that don't bare their own module.
 */
#ifndef LPR_SRC_DEFINITIONS_HPP
#define LPR_SRC_DEFINITIONS_HPP

#include <cmath>
#include <algorithm>

// A common alias for unsigned integer
typedef unsigned int uint;

// Return the number closer to zero of a or b.
template<class Type> Type abs_min(Type a, Type b){
    if(std::abs(a) < std::abs(b))
        return a;
    return b;
}

#endif
