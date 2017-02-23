/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#ifndef HPPB_SRC_VOLUME_HPP
#define HPPB_SRC_VOLUME_HPP

#include "definitions.hpp"
#include "Point.hpp"

#include <vector>
#include <iostream>

class Cluster;

/**
 * An axis aligned section of 3-space.
 */
struct Volume {
    // A zero area section located at the origin
    Volume() {}
    // A unit area section located at the given point
    Volume(Point p) : offset(p), size{1, 1, 1} {}
    // A section located at the given point with the given size
    Volume(Point p, Size s) : offset(p), size(s) {}

    // Calculate the interior volume of the section
    float volume() const;
    // Calculate the surface area of the section
    float surface() const;
    float linear_dimensions() const;

    // Calculate the lowest points on each axis
    int xmin() const;
    int ymin() const;
    int zmin() const;

    // Calculate the highest points INSIDE of the volume
    // only valid with non-zero areas.
    int xmax() const;
    int ymax() const;
    int zmax() const;

    float center(int) const;

    // A volume has a false value when it has a zero
    // surface area AND volume
    operator bool () const;

    // parts of this not intersecting other
    Cluster operator - (Volume) const;

    // Intersection
    Volume operator & (Volume) const;

    // Union (mutual bounding box)
    Volume operator | (Volume) const;

    // Test for overlap between volumes
    bool overlap(Volume) const;

    // Test if two volumes are adjacent
    bool adjacent(Volume) const;

    // Get the distance between volumes on the given axis
    template<uint> int gap(Volume) const;
    template<uint> int gap(int) const;
    int gap(Volume, int) const;

    // Get the surface area in contact between volumes,
    // may assume that adjacent returns true as a precondition
    template<int axis>
    float contact(Volume, int) const;
    float contact(Volume) const;
    int contact_axis(Volume) const;

    // Check if a point is located inside of this volume
    bool contains(Point) const;
    bool contains(Volume) const;

    Volume grow(int) const;

    //
    Point offset;
    Size size;
};


// Print a volume to an output stream
std::ostream& operator << (std::ostream&, Volume);

#endif
