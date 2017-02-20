/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
/**
 *  Simple types and function definitions that don't bare their own module.
 */
#ifndef HPPB_SRC_DEFINITIONS_HPP
#define HPPB_SRC_DEFINITIONS_HPP

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
