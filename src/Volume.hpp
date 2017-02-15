#ifndef LPR_SRC_VOLUME_HPP
#define LPR_SRC_VOLUME_HPP

#include "definitions.hpp"
#include "Point.hpp"

#include <vector>
#include <iostream>

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

    // Calculate the lowest points on each axis
    int xmin() const;
    int ymin() const;
    int zmin() const;

    // Calculate the highest points INSIDE of the volume
    // only valid with non-zero areas.
    int xmax() const;
    int ymax() const;
    int zmax() const;

    // A volume has a false value when it has a zero
    // surface area AND volume
    operator bool () const;

    // parts of this not intersecting other
    std::vector<Volume> operator - (Volume) const;

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
    int gap(Volume, int) const;

    // Get the surface area in contact between volumes,
    // may assume that adjacent returns true as a precondition
    float contact(Volume) const;

    // Check if a point is located inside of this volume
    bool contains(Point) const;

    //
    Point offset;
    Size size;
};

//
//  In all operations where an assumption that volumes in a set don't
//  overlap makes sense, that assumption is in place.
//

// Return all parts of space in the given set that do not overlap with the
// volume given
std::vector<Volume> operator - (const std::vector<Volume>&, Volume);

// Calculate the external surface area of a set of volumes
float surface(const std::vector<Volume>&);

// Calculate the total volume of a set volumes
float volume(const std::vector<Volume>&);

// Test if a volume is adjacent to any item of the set
bool adjacent(const std::vector<Volume>&, Volume);

// Calculate the surface area in contact between two sets of volumes
float contact(const std::vector<Volume>&, const std::vector<Volume>&);

// Try to remove redundancy or merge volumes where possible
void compact(std::vector<Volume>&);

// Return the set of volumes broken into connected sections
std::vector<std::vector<Volume>> connected_components(std::vector<Volume>);

// Print a volume to an output stream
std::ostream& operator << (std::ostream&, Volume);

#endif
