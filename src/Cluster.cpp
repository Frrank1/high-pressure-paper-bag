/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "Cluster.hpp"

//
//  Ways of constructing/writing clusters
//

Cluster::Cluster() {}
Cluster::Cluster(std::initializer_list<Volume> items)
:   m_volumes(items)
{
    init();
}
Cluster::Cluster(const std::vector<Volume>& items)
:   m_volumes(items)
{
    init();
}
Cluster::Cluster(const std::vector<Volume>&& items)
:   m_volumes(items)
{
    init();
}

Cluster::Cluster(const Cluster& o)
:   m_volumes(o.m_volumes)
,   m_surface(o.m_surface)
,   m_volume(o.m_volume)
,   m_bounds(o.m_bounds)
{}

Cluster& Cluster::operator = (const Cluster& o) {
    m_volumes = o.m_volumes;
    m_surface = o.m_surface;
    m_volume = o.m_volume;
    m_bounds = o.m_bounds;
    return *this;
}

Cluster::Cluster(Cluster&&o)
:   m_volumes(std::move(o.m_volumes))
,   m_surface(o.m_surface)
,   m_volume(o.m_volume)
,   m_bounds(o.m_bounds)
{}

Cluster& Cluster::operator = (Cluster&& o) {
    m_volumes = std::move(o.m_volumes);
    m_surface = o.m_surface;
    m_volume = o.m_volume;
    m_bounds = o.m_bounds;
    return *this;
}

void Cluster::init(){
    decltype(m_volumes) temp;
    std::swap(temp, m_volumes);
    m_surface = m_volume = 0;
    for(auto part : temp)
        add(part);
}


//
//  A cluster should act as a sequence
//

Cluster::const_iterator Cluster::begin() const {
    return Cluster::const_iterator(*this, 0);
}

Cluster::const_iterator Cluster::end() const {
    return Cluster::const_iterator(*this, size());
}

Volume Cluster::operator[](uint index) const {
    return m_volumes[index];
}

//
//      Trivial read operations
//

bool Cluster::empty() const {
    return m_volumes.empty();
}

size_t Cluster::size() const {
    return m_volumes.size();
}

bool Cluster::operator == (const Cluster& o) const {
    // TODO this isn't really accurate
    return m_volumes == o.m_volumes;
}

bool Cluster::operator != (const Cluster& o) const {
    // TODO this isn't really accurate
    return m_volumes != o.m_volumes;
}

const std::vector<Volume>& Cluster::parts() const {
    return m_volumes;
}

float Cluster::surface() const {
    return m_surface;
}

Volume Cluster::bounds() const {
    return m_bounds;
}

float Cluster::volume() const {
    return m_volume;
}

//
//      Methods for modifying the cluster
//

void Cluster::add(Volume new_part){
    // All of this assumes the new part doesn't overlap with
    // any part of the chunk

    // Update the surface, add the surface of the new part,
    // remove both sides of the contact of the new part with the
    m_surface += new_part.surface();
    for(auto part : m_volumes){
        m_surface -= 2.0 * new_part.contact(part);
    }

    // Expand the bounds to include the new part
    if(m_volumes.empty())
        m_bounds = new_part;
    else
        m_bounds = m_bounds | new_part;

    // add in the new volume
    m_volume += new_part.volume();

    // Add the new volume to the list
    m_volumes.push_back(new_part);
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
void Cluster::compact(){
    // Remove overlapping parts
    for(int ii = 0; ii < int(m_volumes.size()); ii++){
        for(int jj = ii + 1; jj < int(m_volumes.size()); jj++){
            if(m_volumes[ii].overlap(m_volumes[jj])){
                for(auto part : m_volumes[jj] - m_volumes[ii])
                    if(part.volume() > 0)
                        m_volumes.push_back(part);
                remove(m_volumes, jj);
                jj--;
            }
        }
    }

    // This merges the parts of components
    bool improving = true;
    while(improving){
        improving = false;
        for(int ii = 0; ii < int(m_volumes.size()); ii++){
            for(int jj = ii + 1; jj < int(m_volumes.size()); jj++){
                if(can_merge(m_volumes[ii], m_volumes[jj])){
                    m_volumes[ii] = merge_volumes(m_volumes[ii], m_volumes[jj]);
                    remove(m_volumes, jj);
                    ii--;
                    improving = true;
                    break;
                }
            }
        }
    }
    init();
}


//
//      More complex operations on clusters
//

Cluster Cluster::operator - (Volume o) const {
    std::vector<Volume> out;
    for(auto item : m_volumes){
        for(auto sub : item - o)
            if(sub.volume() > 0)
                out.push_back(sub);
    }
    return out;
}

Cluster Cluster::operator & (Volume o) const {
    std::vector<Volume> out;
    for(auto item : m_volumes){
        auto sub = item & o;
        if(sub.volume() > 0)
            out.push_back(sub);
    }
    return out;
}

bool Cluster::adjacent(Volume other) const {
    for(auto part : m_volumes){
        if(part.adjacent(other))
            return true;
    }
    return false;
}

float Cluster::contact(const Cluster& b) const {
    float out = 0;
    for(auto a_part : m_volumes){
        for(auto b_part : b){
            out += a_part.contact(b_part);
        }
    }
    return out;
}

std::vector<Cluster> Cluster::connected_components() const {
    auto input = m_volumes;
    std::vector<Cluster> output;
    std::vector<Volume> current = {input.back()};
    input.pop_back();

    auto adjacent = [](std::vector<Volume>& base, Volume other) -> bool {
        for(auto part : base){
            if(part.adjacent(other))
                return true;
        }
        return false;
    };

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

//
//      Iterator implementation
//

Cluster::const_iterator::const_iterator(const Cluster& s, int i)
:   source(s), index(i) {}

auto Cluster::const_iterator::operator++() -> const_iterator& {
    index++;
    return *this;
}

auto Cluster::const_iterator::operator++(int) -> const_iterator {
    auto temp = *this;
    index++;
    return temp;
}

bool Cluster::const_iterator::operator==(const_iterator other) {
    return index == other.index;
}
bool Cluster::const_iterator::operator!=(const_iterator other) {
    return index != other.index;
}
auto Cluster::const_iterator::operator*() const -> value_type {
    return source[index];
}


//
//
//

std::ostream& operator << (std::ostream& stream, const Cluster& data){
    stream << "[";
    for(auto part : data)
        stream << part << " ";
    stream << "]";
    return stream;
}
