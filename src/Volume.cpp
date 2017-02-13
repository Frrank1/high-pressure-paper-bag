#include "Volume.hpp"

//
//  Trivially read operations
//

float Volume::volume() const {
    return size.x * size.y * size.z;
}

float Volume::surface() const {
    return 2*(size.x * size.y + size.x * size.z + size.y * size.z);
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
    return offset.x + size.x;
}

int Volume::ymax() const {
    return offset.y + size.y;
}

int Volume::zmax() const {
    return offset.z + size.z;
}

Volume::operator bool () const {
    return !(size.x == 0 and size.y == 0 and size.z == 0);
}

//
//  Set operations on blocks of space
//

// parts of this not intersecting other
std::vector<Volume> Volume::operator - (Volume o) const {
    std::vector<Volume> out;

    // Calculate the first and last blocks of the middle section
    // (if it exists x_mid_start <= x_mid_end)
    int x_mid_start = std::max(offset.x, o.offset.x);
    int x_mid_end = std::min(xmax(), o.xmax());

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
            Point{x_mid_end, offset.y, offset.z},
            Size(std::min(uint(xmax() - o.xmax()), size.x), size.y, size.z)
        ));
    }

    // Try to find some bits where they overlap on the x axis
    if(x_mid_start <= x_mid_end){
        uint x_width = x_mid_end - x_mid_start;

        // Move to the next
        int y_mid_start = std::max(offset.y, o.offset.y);
        int y_mid_end = std::min(ymax(), o.ymax());


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
            int z_mid_end = std::min(zmax(), o.zmax());

            // We have an area left of them
            if(offset.z < o.offset.z){
                out.push_back(Volume(
                    Point{x_mid_start, y_mid_start, offset.z},
                    Size(x_width, y_width, std::min(uint(o.offset.z - offset.z), size.z))
                ));
            }

            // we have an area right of them
            if(ymax() > o.ymax()){
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
        std::min(xmax(), o.xmax()) - point.x,
        std::min(ymax(), o.ymax()) - point.y,
        std::min(zmax(), o.zmax()) - point.z
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
        std::max(xmax(), o.xmax()) + 1 - point.x,
        std::max(ymax(), o.ymax()) + 1 - point.y,
        std::max(zmax(), o.zmax()) + 1 - point.z
    );

    return Volume(point, size);
}

//
//      Comparisons used
//

bool Volume::overlap(Volume o) const{
    return !(
        o.xmax() <= offset.x || xmax() <= o.offset.x ||
        o.ymax() <= offset.y || ymax() <= o.offset.y ||
        o.zmax() <= offset.z || zmax() <= o.offset.z
    );
}

bool Volume::adjacent(Volume o) const {
    bool out = false;
    out |= (gap(o, 0) == 0 && gap(o, 1) < 0 && gap(o, 2) < 0);
    out |= (gap(o, 1) == 0 && gap(o, 2) < 0 && gap(o, 0) < 0);
    out |= (gap(o, 2) == 0 && gap(o, 0) < 0 && gap(o, 1) < 0);
    return out;
}

// Calculate the gap on an axis by taking the max of the min edge - max edge
int Volume::gap(Volume o, int dim) const{
    return std::max(
        offset[dim] - int(o.offset[dim] + o.size[dim]),
        o.offset[dim] - int(offset[dim] + size[dim])
    );
}

float Volume::contact(Volume o) const{
    for(auto axis : {0, 1, 2}){
        if(gap(o, axis) == 0){
            return gap(o, (axis + 1)%3) * gap(o, (axis + 2)%3);
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

//
//      Operations over sets of volumes
//

std::vector<Volume> operator - (const std::vector<Volume>& base, Volume o){
    std::vector<Volume> out;
    for(auto item : base){
        for(auto sub : item - o)
            if(sub.volume() > 0)
                out.push_back(sub);
    }
    return out;
}

// Assumes the volumes are non-overlapping
float surface(const std::vector<Volume>& base){
    float out = 0;
    for(uint ii = 0; ii < base.size(); ii++){
        out += base[ii].surface();
        for(uint jj = ii + 1; jj < base.size(); jj++){
            out -= 2*base[ii].contact(base[jj]);
        }
    }
    return out;
}

// Assumes the volumes are non-overlapping
float volume(const std::vector<Volume>& collection) {
    float out = 0;
    for(auto volume : collection){
        out += volume.volume();
    }
    return out;
}

float contact(const std::vector<Volume>& a, const std::vector<Volume>& b){
    float out = 0;
    for(auto a_part : a){
        for(auto b_part : b){
            out += a_part.contact(b_part);
        }
    }
    return out;
}

// Remove redundant components and merge all volumes togeather
void compact(std::vector<Volume>& /*collection*/){
    #warning "compact is null op"
}

std::ostream& operator << (std::ostream& out, Volume item){
    out << "<(" << item.xmin() << ", " << item.xmax() << ") ("
        << item.ymin() << ", " << item.ymax() << ") ("
        << item.zmin() << ", " << item.zmax() << ")>";
    return out;
}
