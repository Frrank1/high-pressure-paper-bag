/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

#include "GasGraph.hpp"

int main(){
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    GasGraph graph(0);

    std::vector<GasGraph::Node*> nodes;

    for(int ii = 0; ii < 10; ii++){
        auto node = graph.new_node();
        nodes.push_back(node);
        node->volume = 1;
        node->surface = 6;
    }

    for(uint ii = 0; ii < nodes.size() - 1; ii++){
        graph.set_edge(nodes[ii], nodes[ii + 1], 1);
    }

    nodes[4]->gas_mass = 1;

    int steps = 2*128;
    for(int ii = 0; ii < steps/2; ii++){
        std::cout.precision(3);
        for(auto& node : nodes)
            std::cout << std::setw(10) << node->gas_mass;
        std::cout << std::endl;
        // for(int jj = 0; jj < 10; jj++)
        graph.step(0.001);

    }

    for(int ii = 0; ii < steps/2; ii++){
        nodes[0]->gas_mass = 0;
        std::cout.precision(3);
        for(auto& node : nodes)
            std::cout << std::setw(10) << node->gas_mass;
        std::cout << std::endl;

        // for(int jj = 0; jj < 10; jj++)
        graph.step(0.001);

    }

    float check = 0;
    for(auto node : nodes) check += node->gas_mass;

    auto end = clock.now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Final total " << check << std::endl;
    std::cout << "Time " << elapsed_seconds.count() << " "  << elapsed_seconds.count()/steps << std::endl;
    return 0;
}
