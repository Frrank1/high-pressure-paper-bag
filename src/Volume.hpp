#ifndef LPR_SRC_VOLUME_HPP
#define LPR_SRC_VOLUME_HPP

#include "definitions.hpp"
#include "Point.hpp"

#include <vector>
#include <iostream>

struct Volume {
    Volume() {}
    Volume(Point p) : offset(p), size{1, 1, 1} {}
    Volume(Point p, Size s) : offset(p), size(s) {}

    float volume() const;
    float surface() const;

    int xmin() const;
    int ymin() const;
    int zmin() const;

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

    // Is this and the other intersecting or
    // one unit from being adjacent
    bool overlap(Volume) const;
    bool adjacent(Volume) const;
    template<uint> int gap(Volume) const;
    int gap(Volume, int) const;
    float contact(Volume) const;
    bool contains(Point) const;

    Point offset;
    Size size;
};

std::vector<Volume> operator - (const std::vector<Volume>&, Volume);
float surface(const std::vector<Volume>&);
float volume(const std::vector<Volume>&);
bool adjacent(const std::vector<Volume>&, Volume);
float contact(const std::vector<Volume>&, const std::vector<Volume>&);
void compact(std::vector<Volume>&);
std::vector<std::vector<Volume>> connected_components(std::vector<Volume>);
std::ostream& operator << (std::ostream&, Volume);

#endif
