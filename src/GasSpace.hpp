#ifndef HPPB_SRC_GASSPACE_HPP
#define HPPB_SRC_GASSPACE_HPP

#include "definitions.hpp"

#include "GasGraph.hpp"
#include "Volume.hpp"

#include <tuple>

/**
 * Manage the mapping from 3d integer space to a GasGraph.
 */
class GasSpace {
public:
    // An irregular, but connected, cluster of volumes that are represented
    // by a single node in the gas graph.
    struct Sector {
        GasGraph::Node * node;
        std::vector<Volume> parts;
        bool adjacent(Volume) const;
        Volume bounds() const;
    };

public:
    // Construct/Destruct
    GasSpace();
    ~GasSpace();

public:
    // Let the gas flow between the nodes a bit.
    void step(float);

public:
    // Declare that a section of space is not passible to gas.
    void block(Volume);
    // Declare that a section of space is passible to gas.
    void clear(Volume);

    // Measure how much gas is at a point in space
    float air_at(Point) const;
    // Add gas to a point in space.
    // If this point is not passible to gas nothing happens.
    void add_air(Point, float);

public:
    // How many sectors is the space broken into.
    uint size() const;
    // Give a string describing the structure of the sectors.
    std::string describe() const;

protected:
    // There should be at most one sector covering a given point,
    // find that sector or null
    Sector * find_sector(Point) const;

    // Get the sectors that overlap with the given volume
    std::vector<Sector*> overlapping_sectors(Volume) const;

    // Get the sectors that have some surface in contact with
    // the given volume or sector
    std::vector<Sector*> adjacent_sectors(Volume) const;
    std::vector<Sector*> adjacent_sectors(Sector*) const;

    // Get all the sectors overlapping or adjacent to the given volume.
    std::vector<Sector*> affected_sectors(Volume) const;

protected:
    // Create a new sector covering the given space
    Sector* create_sector(Volume);
    Sector* create_sector(const std::vector<Volume>&);

    // Remove a sector from the problem
    void remove_sector(Sector*);

    // Add a volume to a sector
    void expand(Sector*, Volume);

    // Break a sector if needed
    void partition_sector(Sector*);

protected:
    // Update the surface area and volume information in the Gas Graph node.
    void update_node(Sector*);
    // Update the adjacency information between this node and its neighbours.
    void update_adjacency(Sector*);

protected:
    // Check if there is a part of the given volume that this sector
    // would like to have, if so say how much
    std::tuple<float, Volume> choose_addition(Sector*, Volume) const;

    // Measure the effect on the quality of the sector by the addition
    // of the given volume
    float score_addition(Sector*, Volume) const;
    // Minimal reasonable value for score_addition
    const float score_threshold = 1;

protected:
    // Underlying graph that manages update to gas levels
    GasGraph m_graph;

    // List of all sectors in no particular order
    std::vector<Sector*> m_sectors;
};

#endif
