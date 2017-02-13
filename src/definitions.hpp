#ifndef LPR_SRC_DEFINITIONS_HPP
#define LPR_SRC_DEFINITIONS_HPP

#include <cmath>
#include <algorithm>

typedef unsigned int uint;

template<class Type> Type abs_min(Type a, Type b){
    if(std::abs(a) < std::abs(b))
        return a;
    return b;
}

#endif
