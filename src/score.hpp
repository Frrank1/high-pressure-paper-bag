/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#ifndef HPPB_SCORE_FUNCTION_HPP
#define HPPB_SCORE_FUNCTION_HPP

#include <vector>
class Volume;

struct Split {
    float score;
    int index;
    int axis;
};

Split score(const std::vector<Volume>& shape);

#endif
