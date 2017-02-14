#ifndef LPR_SRC_GASGRAPH_HPP
#define LPR_SRC_GASGRAPH_HPP

#include "definitions.hpp"

#include <vector>

class GasGraph {
public:
    struct Edge;

    struct Node {
        float gas = 0;
        float volume = 0;
        float surface = 0;

        std::vector<Edge> edges;

        float density() const;
    };

    struct Edge {
        float surface;
        Node * other;
    };

public:
    GasGraph();
    ~GasGraph();

public:
    void step(float delta);
    void step_node(Node*, float delta);

public:
    Node * new_node();
    void release_node(Node*);
    void merge_nodes(Node*, Node*);

    void set_edge(Node*, Node*, float);
    void clear_edges(Node*);
    void clear_edge(Node*, Node*);

protected:
    std::vector<Node*> m_nodes;
};

#endif
