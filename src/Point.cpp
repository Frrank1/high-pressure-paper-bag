/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "Point.hpp"

Size::Size()
:   x(0), y(0), z(0) {}

Size::Size(int _x, int _y, int _z)
:   x(std::max(0, _x))
,   y(std::max(0, _y))
,   z(std::max(0, _z)) {}

Size::Size(uint _x, uint _y, uint _z)
:   x(_x), y(_y), z(_z) {}

bool Size::operator == (Size o) const {
    return x == o.x && y == o.y && z == o.z;
}


Point::Point()
:   x(0), y(0), z(0) {}

Point::Point(int _x, int _y, int _z)
:   x(_x), y(_y), z(_z) {}

bool Point::operator == (Point o) const {
    return x == o.x && y == o.y && z == o.z;
}
