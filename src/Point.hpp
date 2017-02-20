/**
 * Simple objects for tracking distances and positions in 3D space.
 *
 * If these classes get any more complex I'm tempted to make this into
 * a template.
 */
#ifndef HPPB_SRC_POINT_HPP
#define HPPB_SRC_POINT_HPP

#include "definitions.hpp"

/**
 * The size of an axis alligned chunk of 3-space.
 */
struct Size {
    // Default construct zero sized chunk
    Size();
    // Create a size of the given dimensions (or zero for negative values)
    Size(int, int, int);
    // Create a size of the given dimensions
    Size(uint, uint, uint);

    // Create three integers that can be referred to by name or index.
    union {
        uint data[3];
        struct {
            uint x;
            uint y;
            uint z;
        };
    };

    // Get one of the three integers by index
    uint operator[](unsigned int index) const {
        return data[index];
    }
    uint& operator[](unsigned int index) {
        return data[index];
    }

    // Compare two volumes
    bool operator == (Size) const;
};

/**
 * A point in 3-space.
 */
struct Point {
    // A point at the origin in 3-space.
    Point();
    // Load a point from values
    Point(int, int, int);

    // Create three integers that can be referred to by name or index.
    union {
        int data[3];
        struct {
            int x;
            int y;
            int z;
        };
    };

    // Get one of the three integers by index
    int operator[](unsigned int index) const {
        return data[index];
    }
    int& operator[](unsigned int index) {
        return data[index];
    }

    // Compare two points
    bool operator == (Point) const;
};

#endif
