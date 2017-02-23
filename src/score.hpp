/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
/**
 * A heuristic for measuring if a cluster of volumes should be split; and
 * if so, where.
 *
 * Iterates over the seams/edges of volumes and tests how many objects/internal
 * faces would be cut by breaking the cluster at that time.
 */
#ifndef HPPB_SCORE_FUNCTION_HPP
#define HPPB_SCORE_FUNCTION_HPP

#include <vector>
class Volume;
class Cluster;

struct Split {
    // Quality of the shape before splitting.
    // Numbers lower than 1.0 mean that the split is actively recommended.
    // For numbers greather than or equal 1.0 a split is returned, but it
    // may not actually improve the situation.
    // A score of infinity means that there is actually no split worth returning.
    float score;
    // For non-infinity scores this index and axis defines a plane that could
    // be used to split the given shape.
    int index;
    int axis;
};

// Measure the given collection of volumes and see if there is a reasonable
// place to split.
Split score(const std::vector<Volume>& shape);

#endif
