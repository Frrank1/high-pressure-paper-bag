/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include <iostream>
#include <vector>

#include "GasGraph.hpp"

int main(){

    GasGraph graph;

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

    nodes[0]->gas = 1;

    for(int ii = 0; ii < 10; ii++){
        for(auto& node : nodes)
            std::cout << node->gas << " ";
        std::cout << std::endl;

        for(int ii = 0; ii < 5; ii++)
            graph.step(0.2);
    }

    float check = 0;
    for(auto node : nodes) check += node->gas;
    std::cout << "Final total " << check << std::endl;

    return 0;
}
