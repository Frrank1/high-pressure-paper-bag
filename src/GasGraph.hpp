/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#ifndef HPPB_SRC_GASGRAPH_HPP
#define HPPB_SRC_GASGRAPH_HPP

#include "definitions.hpp"
#include <vector>
#include <random>

/**
 * Manages a very simple network of gas bubbles with gas moving between
 * them. The simulation is not actually based on any physical model.
 */
class GasGraph {
public:
    // Forward declaration to resolve dependence.
    struct Edge;

    // Node in the gas graph
    struct Node {
        float gas_mass = 0;

        // Track the size and shape of the node
        float volume = 0;
        float surface = 0;

        // Track the neighbours of the node
        std::vector<Edge> edges;

        // Simple method to calculate density of gas in this pocket
        float density() const;
        float pressure() const;
    };

    // An edge between nodes
    struct Edge {
        // How much surface area is shared between the parent of this edge
        // and the other node
        float surface;
        Node * other;

        // TODO Needs to be symetric
        float acceleration = 0;
        float velocity = 0;
    };

public:
    // Construct/Deconstruct
    GasGraph(uint);
    ~GasGraph();

public:
    // Allow gas pressure to equalize between connected nodes
    void step(float delta);

protected:
    // Perform the equalization step for one node
    void step_node(Node*, float delta);

public:
    // Create a new node
    Node * new_node();
    // Remove a node from the graph
    void remove_node(Node*);
    // Merge two nodes into a single one.
    void merge_nodes(Node*, Node*);

    // Set the surface area connecting two nodes
    void set_edge(Node*, Node*, float);

    // Remove all of the edges connecting a node to its neighbours
    void clear_edges(Node*);

    // Remove a connection between nodes
    void clear_edge(Node*, Node*);

protected:
    std::vector<Node*> m_nodes;
    const float almost_nothing = 1e-4;
    std::mt19937 m_prng;

};

#endif
