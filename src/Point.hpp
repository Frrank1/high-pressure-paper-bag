#ifndef LPR_SRC_POINT_HPP
#define LPR_SRC_POINT_HPP

#include "definitions.hpp"

struct Size {
    Size();
    Size(int, int, int);
    Size(uint, uint, uint);

    union {
        uint data[3];
        struct {
            uint x;
            uint y;
            uint z;
        };
    };

    uint operator[](unsigned int index) const {
        return data[index];
    }
    uint& operator[](unsigned int index) {
        return data[index];
    }

    bool operator == (Size) const;
};

struct Point {
    Point();
    Point(int, int, int);

    union {
        int data[3];
        struct {
            int x;
            int y;
            int z;
        };
    };

    int operator[](unsigned int index) const {
        return data[index];
    }
    int& operator[](unsigned int index) {
        return data[index];
    }

    bool operator == (Point) const;
};

#endif
