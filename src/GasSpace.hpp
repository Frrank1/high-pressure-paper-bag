#ifndef LPR_SRC_GASSPACE_HPP
#define LPR_SRC_GASSPACE_HPP

#include "definitions.hpp"

#include "GasGraph.hpp"
#include "Volume.hpp"

#include <tuple>


class GasSpace {
public:
    struct Sector {
        GasGraph::Node * node;
        std::vector<Volume> parts;
        bool adjacent(Volume) const;
        Volume bounds() const;
    };

public:
    GasSpace();
    ~GasSpace();

public:
    void step(float);

public:
    void block(Volume);
    void clear(Volume);

    float air_at(Point) const;
    void add_air(Point, float);

public:
    uint size() const;
    std::string describe() const;

protected:
    Sector * find_sector(Point) const;
    std::vector<Sector*> overlapping_sectors(Volume) const;
    std::vector<Sector*> adjacent_sectors(Volume) const;
    std::vector<Sector*> adjacent_sectors(Sector*) const;
    std::vector<Sector*> affected_sectors(Volume) const;

protected:
    void create_sector(Volume);
    void expand(Sector*, Volume);
    void remove(Sector*, Volume);

    void update_node(Sector*);
    void update_adjacency(Sector*);

protected:
    std::tuple<float, Volume> choose_addition(Sector*, Volume) const;
    float score_addition(Sector*, Volume) const;

protected:
    GasGraph m_graph;
    std::vector<Sector*> m_sectors;
};

#endif
