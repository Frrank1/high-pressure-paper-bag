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
    return offset.x + size.x - 1;
}

int Volume::ymax() const {
    return offset.y + size.y - 1;
}

int Volume::zmax() const {
    return offset.z + size.z - 1;
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
std::vector<Volume> Volume::operator - (Volume o) const {
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
        std::max(xmax(), o.xmax()) - point.x + 2,
        std::max(ymax(), o.ymax()) - point.y + 2,
        std::max(zmax(), o.zmax()) - point.z + 2
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

int Volume::gap(Volume o, int dim) const{
    return std::max(
        offset[dim] - int(o.offset[dim] + o.size[dim]),
        o.offset[dim] - int(offset[dim] + size[dim])
    );
}

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
            out -= 2.0 * base[ii].contact(base[jj]);
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

bool adjacent(const std::vector<Volume>& base, Volume other){
    for(auto part : base){
        if(part.adjacent(other))
            return true;
    }
    return false;
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

namespace {
    // Helper functions for the compact function
    bool can_merge(Volume a, Volume b){
        bool dim_x = a.offset.x == b.offset.x && a.size.x == b.size.x;
        bool dim_y = a.offset.y == b.offset.y && a.size.y == b.size.y;
        bool dim_z = a.offset.z == b.offset.z && a.size.z == b.size.z;
        return (dim_x && dim_y && a.gap<2>(b) == 0)
            || (dim_x && dim_z && a.gap<1>(b) == 0)
            || (dim_y && dim_z && a.gap<0>(b) == 0);
    }

    Volume merge_volumes(Volume a, Volume b){
        bool dim_x = a.offset.x == b.offset.x && a.size.x == b.size.x;
        bool dim_y = a.offset.y == b.offset.y && a.size.y == b.size.y;
        bool dim_z = a.offset.z == b.offset.z && a.size.z == b.size.z;
        if(dim_x && dim_y && a.gap<2>(b) == 0){
            return Volume({a.offset.x, a.offset.y, std::min(a.offset.z, b.offset.z)},
                          {a.size.x, a.size.y, a.size.z + b.size.z});
        } else if(dim_x && dim_z && a.gap<1>(b) == 0){
            return Volume({a.offset.x, std::min(a.offset.y, b.offset.y), a.offset.z},
                          {a.size.x, a.size.y + b.size.y, a.size.z});
        } else if(dim_y && dim_z && a.gap<0>(b) == 0){
            return Volume({std::min(a.offset.x, b.offset.x), a.offset.y, a.offset.z},
                          {a.size.x + b.size.x, a.size.y, a.size.z});
        }
        return a;
    }

    void remove(std::vector<Volume>& collection, int index){
        std::swap(collection[index], collection.back());
        collection.pop_back();
    }
};

// Remove redundant components and merge all volumes togeather
// Could use a space partitioning structure to make this faster/better
void compact(std::vector<Volume>& collection){
    // Remove overlapping parts
    for(int ii = 0; ii < int(collection.size()); ii++){
        for(int jj = ii + 1; jj < int(collection.size()); jj++){
            if(collection[ii].overlap(collection[jj])){
                for(auto part : collection[jj] - collection[ii])
                    if(part.volume() > 0)
                        collection.push_back(part);
                remove(collection, jj);
                jj--;
            }
        }
    }

    // This merges the parts of components
    bool improving = true;
    while(improving){
        improving = false;
        for(int ii = 0; ii < int(collection.size()); ii++){
            for(int jj = ii + 1; jj < int(collection.size()); jj++){
                if(can_merge(collection[ii], collection[jj])){
                    collection[ii] = merge_volumes(collection[ii], collection[jj]);
                    remove(collection, jj);
                    ii--;
                    improving = true;
                    break;
                }
            }
        }
    }
}

std::vector<std::vector<Volume>> connected_components(std::vector<Volume> input){
    std::vector<std::vector<Volume>> output;
    std::vector<Volume> current = {input.back()};
    input.pop_back();

    while(input.size() > 0){
        int found = -1;
        for(uint ii = 0; ii < input.size(); ii++){
            if(adjacent(current, input[ii])){
                found = ii;
                break;
            }
        }

        if(found == -1){
            output.push_back(current);
            current = {input.back()};
            input.pop_back();
        } else {
            current.push_back(input[found]);
            std::swap(input[found], input.back());
            input.pop_back();
        }
    }

    if(current.size() > 0)
        output.push_back(current);

    return output;
}

std::ostream& operator << (std::ostream& out, Volume item){
    out << "<(" << item.xmin() << ", " << item.xmax() << ") ("
        << item.ymin() << ", " << item.ymax() << ") ("
        << item.zmin() << ", " << item.zmax() << ")>";
    return out;
}
