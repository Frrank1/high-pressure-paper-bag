/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "Volume.hpp"
#include "Cluster.hpp"

//
//  Trivially read operations
//

float Volume::volume() const {
    return size.x * size.y * size.z;
}

float Volume::surface() const {
    return 2*(size.x * size.y + size.x * size.z + size.y * size.z);
}

float Volume::linear_dimensions() const {
    return size.x + size.y + size.z;
}

int Volume::xmin() const {
    return offset.x;
}

int Volume::ymin() const {
    return offset.y;
}

int Volume::zmin() const {
    return offset.z;
}

int Volume::xmax() const {
    return offset.x + size.x - 1;
}

int Volume::ymax() const {
    return offset.y + size.y - 1;
}

int Volume::zmax() const {
    return offset.z + size.z - 1;
}

float Volume::center(int axis) const {
    return offset[axis] + float(size[axis])/2.0f;
}

Volume::operator bool () const {
    return !(size.x == 0 and size.y == 0 and size.z == 0);
}

//
//  Set operations on blocks of space
//

namespace{
auto& debug = std::cout;
}


// std::vector<Volume> Volume::operator - (Volume o) const {
//     std::vector<Volume> out;
//
//     // Calculate the first and last blocks of the middle section
//     // (if it exists x_mid_start <= x_mid_end)
//     int x_mid_start = std::max(offset.x, o.offset.x);
//     int x_mid_end = std::min(xmax(), o.xmax()) + 1;
//
//     // We have an area left of them
//     if(offset.x < o.offset.x){
//         out.push_back(Volume(
//             offset,
//             Size(std::min(uint(o.offset.x - offset.x), size.x), size.y, size.z)
//         ));
//     }
//
//     // we have an area right of them
//     if(xmax() > o.xmax()){
//         out.push_back(Volume(
//             Point{x_mid_end, offset.y, offset.z},
//             Size(std::min(uint(xmax() - o.xmax()), size.x), size.y, size.z)
//         ));
//     }
// }

// parts of this not intersecting other
// TODO refactor
Cluster Volume::operator - (Volume o) const {
    if(!overlap(o)) return {*this};
    std::vector<Volume> out;

    // Calculate the first and last blocks of the middle section
    // (if it exists x_mid_start <= x_mid_end)
    int x_mid_start = std::max(offset.x, o.offset.x);
    int x_mid_end = std::min(xmax(), o.xmax()) + 1;

    // We have an area left of them
    if(offset.x < o.offset.x){
        out.push_back(Volume(
            offset,
            Size(std::min(uint(o.offset.x - offset.x), size.x), size.y, size.z)
        ));
    }

    // we have an area right of them
    if(xmax() > o.xmax()){
        out.push_back(Volume(
            Point{std::max(x_mid_end, offset.x), offset.y, offset.z},
            Size(std::min(uint(xmax() - o.xmax()), size.x), size.y, size.z)
        ));
    }

    // Try to find some bits where they overlap on the x axis
    if(gap<0>(o) < 0){
        uint x_width = x_mid_end - x_mid_start;

        // Move to the next
        int y_mid_start = std::max(offset.y, o.offset.y);
        int y_mid_end = std::min(ymax(), o.ymax()) + 1;


        // We have an area left of them
        if(offset.y < o.offset.y){
            out.push_back(Volume(
                Point{x_mid_start, offset.y, offset.z},
                Size(x_width, std::min(uint(o.offset.y - offset.y), size.y), size.z)
            ));
        }

        // we have an area right of them
        if(ymax() > o.ymax()){
            out.push_back(Volume(
                Point{x_mid_start, y_mid_end, offset.z},
                Size(x_width, std::min(uint(ymax() - o.ymax()), size.y), size.z)
            ));
        }

        if(y_mid_start <= y_mid_end){
            uint y_width = y_mid_end - y_mid_start;

            // Move to the next
            // int z_mid_start = std::max(offset.z, o.offset.z);
            int z_mid_end = std::min(zmax(), o.zmax()) + 1;

            // We have an area left of them
            if(offset.z < o.offset.z){
                out.push_back(Volume(
                    Point{x_mid_start, y_mid_start, offset.z},
                    Size(x_width, y_width, std::min(uint(o.offset.z - offset.z), size.z))
                ));
            }

            // we have an area right of them
            if(zmax() > o.zmax()){
                out.push_back(Volume(
                    Point{x_mid_start, y_mid_start, z_mid_end},
                    Size(x_width, y_width, std::min(uint(zmax() - o.zmax()), size.z))
                ));
            }
        }
    }

    return out;
}


// Intersection
Volume Volume::operator & (Volume o) const {
    Point point {
        std::max(offset.x, o.offset.x),
        std::max(offset.y, o.offset.y),
        std::max(offset.z, o.offset.z)
    };

    Size size (
        std::min(xmax(), o.xmax()) - point.x + 1,
        std::min(ymax(), o.ymax()) - point.y + 1,
        std::min(zmax(), o.zmax()) - point.z + 1
    );

    return Volume(point, size);
}

// Union (mutual bounding box)
Volume Volume::operator | (Volume o) const {
    Point point {
        std::min(offset.x, o.offset.x),
        std::min(offset.y, o.offset.y),
        std::min(offset.z, o.offset.z)
    };

    Size size (
        std::max(xmax(), o.xmax()) - point.x + 1,
        std::max(ymax(), o.ymax()) - point.y + 1,
        std::max(zmax(), o.zmax()) - point.z + 1
    );

    return Volume(point, size);
}

//
//      Comparisons used
//

bool Volume::overlap(Volume o) const{
    return !(
        o.xmax() < offset.x || xmax() < o.offset.x ||
        o.ymax() < offset.y || ymax() < o.offset.y ||
        o.zmax() < offset.z || zmax() < o.offset.z
    );
}

bool Volume::adjacent(Volume o) const {
    bool out = false;
    out |= (gap<0>(o) == 0 && gap<1>(o) < 0 && gap<2>(o) < 0);
    out |= (gap<1>(o) == 0 && gap<2>(o) < 0 && gap<0>(o) < 0);
    out |= (gap<2>(o) == 0 && gap<0>(o) < 0 && gap<1>(o) < 0);
    return out;
}

// Calculate the gap on an axis by taking the max of the min edge - max edge

template<uint dim>
int Volume::gap(Volume o) const{
    return std::max(
        offset[dim] - int(o.offset[dim] + o.size[dim]),
        o.offset[dim] - int(offset[dim] + size[dim])
    );
}

template<uint dim>
int Volume::gap(int ii) const{
    return std::max(
        offset[dim] - ii,
        ii - int(offset[dim] + size[dim])
    );
}

int Volume::gap(Volume o, int dim) const{
    return std::max(
        offset[dim] - int(o.offset[dim] + o.size[dim]),
        o.offset[dim] - int(offset[dim] + size[dim])
    );
}

template<int axis>
float Volume::contact(Volume o, int x) const{
    if(gap<axis>(x) == 0 and o.gap<axis>(x)) return 0;

    const uint ib = (axis + 1) % 3;
    const uint ic = (axis + 2) % 3;

    bool a = gap(o, axis) == 0;

    int b = gap(o, ib);
    int c = gap(o, ic);
    if(a and b < 0 and c < 0){
        return std::min(-b, int(std::min(size[ib], o.size[ib]))) *
            std::min(-c, int(std::min(size[ic], o.size[ic])));
    }
    return 0;
}
template float Volume::contact<0>(Volume, int) const;
template float Volume::contact<1>(Volume, int) const;
template float Volume::contact<2>(Volume, int) const;

float Volume::contact(Volume o) const{
    for(auto axis : {0, 1, 2}){
        uint ib = (axis + 1) % 3;
        uint ic = (axis + 2) % 3;

        bool a = gap(o, axis) == 0;

        int b = gap(o, ib);
        int c = gap(o, ic);
        if(a and b < 0 and c < 0){
            return std::min(-b, int(std::min(size[ib], o.size[ib]))) *
                std::min(-c, int(std::min(size[ic], o.size[ic])));
        }
    }
    return 0;
}

int Volume::contact_axis(Volume o) const{
    for(auto axis : {0, 1, 2}){
        uint ib = (axis + 1) % 3;
        uint ic = (axis + 2) % 3;

        bool a = gap(o, axis) == 0;

        int b = gap(o, ib);
        int c = gap(o, ic);
        if(a and b < 0 and c < 0){
            return a;
        }
    }
    return 0;
}

bool Volume::contains(Point point) const {
    return (
        xmin() <= point.x && point.x <= xmax() &&
        ymin() <= point.y && point.y <= ymax() &&
        zmin() <= point.z && point.z <= zmax()
    );
}

Volume Volume::grow(int distance) const {
    return Volume(Point(
            offset.x - distance,
            offset.y - distance,
            offset.z - distance
        ), Size(
            size.x + distance * 2,
            size.y + distance * 2,
            size.z + distance * 2
        )
    );
}

std::ostream& operator << (std::ostream& out, Volume item){
    out << "<(" << item.xmin() << ", " << item.xmax() << ") ("
        << item.ymin() << ", " << item.ymax() << ") ("
        << item.zmin() << ", " << item.zmax() << ")>";
    return out;
}
