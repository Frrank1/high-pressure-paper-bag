/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "GasSpace.hpp"
#include "score.hpp"
#include "Cluster.hpp"

#include <limits>
#include <unordered_set>
#include <sstream>
#include <iostream>

auto& debug = std::cout;

//
//      Sector operations
//

bool GasSpace::Sector::adjacent(Volume volume) const {
    return parts.adjacent(volume);
}

Volume GasSpace::Sector::bounds() const {
    if(parts.empty()) return Volume();
    return parts.bounds();
}


//
//
//

GasSpace::GasSpace(uint seed) : m_graph(seed) {}

GasSpace::~GasSpace(){
    while(m_sector_list.size() > 0){
        delete m_sector_list.back();
        m_sector_list.pop_back();
    }
}

void GasSpace::step(float delta){
    m_graph.step(delta);
}

void GasSpace::block(Volume volume){
    debug << "Blocking volume " << volume << std::endl;
    std::unordered_set<Sector*> changed_sectors;

    // Modify effected sectors
    for(auto sector : affected_sectors(volume)){
        auto new_parts = sector->parts - volume;

        if(new_parts.volume() == 0){
            debug << "Removing Sector " << sector << std::endl;
            remove_sector(sector);
        } else if(sector->parts != new_parts){
            debug << "Blocking sector" << std::endl;
            for(auto part : sector->parts)
                debug << "-\t" << part << std::endl;
            for(auto part : new_parts)
                debug << "+\t" << part << std::endl;
            sector->parts = new_parts;
            changed_sectors.insert(sector);
            sector->parts.compact();
            update_node(sector);
            update_adjacency(sector);
        }
    }

    // Check for partition
    for(auto sector : changed_sectors){
        partition_sector(sector);
    }
}

void GasSpace::clear(Volume volume){
    debug << "Clearing volume " << volume << std::endl;
    // We may break the volume into parts and give it to multiple sectors
    std::vector<Volume> parts{volume};
    std::vector<Volume> poor_fits;

    // Let sectors take parts of the volume
    while(!parts.empty() or !poor_fits.empty()){
        // If there are no candidate parts, everything is a poor fit
        if(parts.empty() and !poor_fits.empty()){
            // Take one poor fit and make a new sector
            create_sector(poor_fits.back());
            debug << "Adding section as new sector " << poor_fits.back() << std::endl;
            poor_fits.pop_back();

            //
            for(auto bit : poor_fits) parts.push_back(bit);
            poor_fits.clear();
            continue;
        }

        // Take the next part
        auto current = parts.back();
        parts.pop_back();
        debug << "Trying to fit " << current << std::endl;

        // We are going to move through possible sectors and take
        float score = -std::numeric_limits<float>::infinity();
        Volume selected;
        Sector * best_sector = nullptr;

        int attempts = 0;
        for(auto sector : affected_sectors(current)){
            attempts++;
            Volume a;
            float b;
            std::tie(b, a) = choose_addition(sector, current);
            if(b > score){
                score = b;
                selected = a;
                best_sector = sector;
            }
        }
        debug << "Make " << attempts << " attempts to fit." << std::endl;

        if(best_sector and score > score_threshold){
            debug << "expanding " << best_sector << " with " << selected << " (" << score << ")" << std::endl;
            // Put the best section into a sector
            expand(best_sector, selected);

            // put the remaining parts back into the list
            for(auto bit : current - selected){
                parts.push_back(bit);
            }

            // Put the poor fits back in the list, something changing
            // might have made a good place for them
            for(auto bit : poor_fits) parts.push_back(bit);
            poor_fits.clear();
        } else {
            poor_fits.push_back(current);
        }
    }
}
//
//
//

float GasSpace::air_at(Point point) const {
    auto sector = find_sector(point);
    if(sector){
        return sector->node->density();
    }
    return 0;
}

void GasSpace::add_air(Point point, float value){
    auto sector = find_sector(point);
    if(sector){
        sector->node->gas_mass += value;
    }
}

//
//
//

uint GasSpace::size() const {
    return m_sector_list.size();
}

std::string GasSpace::describe() const {
    std::stringstream ss;

    ss << "Size: " << size() << std::endl;

    for(auto sector : m_sector_list){
        ss << "Node " << sector->node << std::endl;
        ss << "Parts " << sector->parts.size() << std::endl;
        for(auto part : sector->parts)
            ss << "\t" << part << std::endl;
        ss << "Neighbours " << sector->node->edges.size() << std::endl;
        for(auto edge : sector->node->edges)
            ss << "\t" << edge.other << "\t" << edge.surface << std::endl;
        ss << std::endl;
    }

    return ss.str();
}

//
//      Methods for finding sectors
//
// Currently brute force, can later be replaced by fast lookup

auto GasSpace::find_sector(Point point) const -> Sector* {
    auto result = m_sector_lookup.intersecting(point);
    if(result.size() > 0)
        return result.front();
    return nullptr;
}

std::vector<GasSpace::Sector*> GasSpace::overlapping_sectors(Volume test) const{
    return m_sector_lookup.intersecting(test);
}

std::vector<GasSpace::Sector*> GasSpace::adjacent_sectors(Volume test) const{
    std::vector<Sector*> out;
    for(auto sector : m_sector_lookup.intersecting(test.grow(1))){
        bool adjacent = false;
        for(auto part : sector->parts){
            if(part.adjacent(test)){
                adjacent = true;
                break;
            }
        }
        if(adjacent)
            out.push_back(sector);
    }
    return out;
}

std::vector<GasSpace::Sector*> GasSpace::adjacent_sectors(Sector* input) const{
    std::unordered_set<Sector*> out;
    for(auto test : input->parts){
        for(auto sector : m_sector_lookup.intersecting(test.grow(1))){
            bool overlap = false;
            for(auto part : sector->parts){
                if(part.adjacent(test)){
                    overlap = true;
                    break;
                }
            }
            if(overlap)
                out.insert(sector);
        }
    }
    return std::vector<Sector*>(out.begin(), out.end());
}

auto GasSpace::affected_sectors(Volume volume) const -> std::vector<Sector*>{
    std::unordered_set<Sector*> out;
    auto adjacent = adjacent_sectors(volume);
    auto overlap = overlapping_sectors(volume);
    out.insert(adjacent.begin(), adjacent.end());
    out.insert(overlap.begin(), overlap.end());
    return std::vector<Sector*>(out.begin(), out.end());
}

//
//      Methods for creating/editing sectors
//

GasSpace::Sector* GasSpace::create_sector(Volume space){
    // Allocate memory
    auto sector = new Sector;
    sector->node = m_graph.new_node();
    sector->node->volume = space.volume();
    sector->node->surface = space.surface();

    // Assign parts
    sector->parts = {space};

    // put in place
    m_sector_list.push_back(sector);
    m_sector_lookup.insert(sector, sector->bounds());
    update_adjacency(sector);
    return sector;
}

GasSpace::Sector* GasSpace::create_sector(const Cluster& space){
    // Allocate memory
    auto sector = new Sector;
    sector->node = m_graph.new_node();
    sector->parts = space;

    // put in place
    m_sector_list.push_back(sector);
    m_sector_lookup.insert(sector, sector->bounds());
    update_node(sector);
    update_adjacency(sector);
    return sector;
}

void GasSpace::remove_sector(Sector* sector){
    // Remove it from the list
    for(uint ii = 0; ii < m_sector_list.size(); ii++){
        if(m_sector_list[ii] == sector){
            std::swap(m_sector_list[ii], m_sector_list.back());
            m_sector_list.pop_back();
        }
    }
    m_sector_lookup.remove(sector);

    // Update the graph
    m_graph.remove_node(sector->node);
}

void GasSpace::expand(Sector* sector, Volume space){
    //
    sector->parts.add(space);
    sector->parts.compact();

    // Update all the aux data
    update_node(sector);
    update_adjacency(sector);
}

// There are two grounds for partitioning a sector:
//   1. It is no longer contiguous, we need a new disconnected node.
//   2. It has components that should actually be modeled as
//      a separate (but connected) node in the graph.
void GasSpace::partition_sector(Sector* sector) {
    debug << "Partitioning: " << std::endl;
    for(auto part : sector->parts)
        debug << "\t" << part << std::endl;

    // Check for the first condition
    auto components = sector->parts.connected_components();
    std::cout << "PARTS " << sector->parts.size() << " " << components.size() << std::endl;
    if(components.size() > 1){
        float old_density = sector->node->density();

        // Reset the base sector
        sector->parts = components.back();
        components.pop_back();
        update_node(sector);
        update_adjacency(sector);
        sector->node->gas_mass = old_density * sector->node->volume;

        // Fill in new components
        std::vector<Sector*> new_sectors = {sector};
        for(auto sub_section : components){
            std::cout << "Subsection" << std::endl;
            auto new_sector = create_sector(sub_section);
            new_sectors.push_back(new_sector);
            new_sector->node->gas_mass = old_density * new_sector->node->volume;
        }

        for(auto sector : new_sectors)
            partition_sector(sector);
        return;
    }

    // Check for the second condition
    Split split = ::score(sector->parts.parts());
    if(split.score < score_threshold){
        auto bounds = sector->parts.bounds();
        auto half_one = bounds;
        half_one.size[split.axis] = split.index - half_one.offset[split.axis];

        auto half_two = bounds;
        half_two.size[split.axis] = bounds.size[split.axis] - half_one.size[split.axis];
        half_two.offset[split.axis] = split.index;

        //
        auto shape_one = sector->parts & half_one;
        auto shape_two = sector->parts & half_two;
        shape_one.compact();
        shape_two.compact();

        // Reset old sector
        sector->parts = shape_one;
        update_node(sector);
        update_adjacency(sector);

        // Create new one
        auto new_sector = create_sector(shape_two);

        // Recurse
        partition_sector(sector);
        partition_sector(new_sector);
    }
}

//
//
//

void GasSpace::update_node(Sector* sector){
    //
    sector->node->volume = sector->parts.volume();
    sector->node->surface = sector->parts.surface();
    std::cout << "new data " << sector->node->volume
        << " " << sector->node->surface << std::endl;
}

void GasSpace::update_adjacency(Sector* sector){
    // Clear existing adjacencies
    m_graph.clear_edges(sector->node);

    for(auto other : adjacent_sectors(sector)){
        if(other == sector) continue;
        // Calculate the contact area between the sectors
        float new_contact = sector->parts.contact(other->parts);
        m_graph.set_edge(sector->node, other->node, new_contact);
    }
}


//
//      Measuring sectors
//

std::tuple<float, Volume> GasSpace::choose_addition(Sector* sector, Volume input) const{
    // If there is a section of the input that overlaps with part of this
    // sector, take it with priority (inf score)
    for(auto part : sector->parts){
        if(part.overlap(input)){
            return std::make_pair(std::numeric_limits<float>::infinity(), part & input);
        }
    }

    // If the vector isn't touching an existing volume we don't want this
    // input at all, reject totally (-inf score)
    if(not sector->adjacent(input))
        return std::make_pair(-std::numeric_limits<float>::infinity(), Volume());

    // Check if there is a segment of the input that
    // can be taken without expanding the bounding box?
    auto bounding_box = sector->bounds();
    auto preferred = bounding_box & input;
    if(preferred.volume() > 0){
        // Measure the increase in surface area for the preferred section
        return std::make_pair(score_addition(sector, preferred), preferred);
    }

    // Measure the increase in surface area, and the increase in negative
    // space within the new bounding box
    return std::make_pair(score_addition(sector, input), input);
}

float GasSpace::score_addition(Sector* sector, Volume input) const {
    std::vector<Volume> new_volumes = sector->parts.parts();
    new_volumes.push_back(input);
    return ::score(new_volumes).score;
}
