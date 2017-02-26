/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "GasGraph.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <fstream>

namespace {
    std::ofstream debug;
}

//
//  Define some constants we will be using
//

constexpr float temperature = 20; // degrees C
constexpr float temperature_kelvin = temperature + 273.15;
constexpr float ideal_gas_constant = 8.3144; // J / (K mol)
constexpr float molar_mass_air = 0.029; // kg/mol
constexpr float specific_constant_air = ideal_gas_constant/molar_mass_air; // J/(kg K)
constexpr float air_viscosity = 0.01; // 1.81e-5; // kg/(m*s)

//
//
//

float GasGraph::Node::density() const {
    return gas_mass / volume;
}

float GasGraph::Node::pressure() const {
    return density() * specific_constant_air * temperature_kelvin;
}

//
//
//

GasGraph::GasGraph(uint seed) : m_prng(seed){}
GasGraph::~GasGraph(){
    for(auto node : m_nodes) delete node;
}

void GasGraph::step(float delta){
    // We are going to make an effort to not process connected nodes in sequence.
    std::unordered_set<Node*> remaining(m_nodes.begin(), m_nodes.end());

    // Keep going until all the nodes get done
    while(!remaining.empty()){
        // For each round of stepping nodes we will avoid those adjacent to
        // those already steppeed in this round.
        std::unordered_set<Node*> adjacent;

        // Each pass should see the nodes in a different order
        auto copy = std::vector<Node*>(remaining.begin(), remaining.end());
        std::shuffle(copy.begin(), copy.end(), m_prng);

        for(auto node : copy) {
            // Skip nodes adjacent to those already done until the next pass
            if(adjacent.count(node)) continue;

            // Do the node
            step_node(node, delta);
            remaining.erase(node);

            // forbid the neighbours until the next round
            for(auto edge : node->edges)
                adjacent.insert(edge.other);
        }
    }
}

void GasGraph::step_node(Node* node, float delta_time){
    // If the node has no space or is disconnected we can stop early
    if(node->volume == 0) return;
    if(node->edges.empty()) return;

    // We'll calculate these values for each connection with another node
    std::vector<float> mass_flows;
    std::vector<float> acceleration, velocity;

    // Use the pressure gradient on each opening to the node to calculate the gas
    // flow in/out
    for(auto& edge : node->edges){
        auto other = edge.other;

        // If the other node has no volume or both this and the other node
        // are nearly empty, then we can assume nothing interesting is happening here
        if(other->volume == 0 || (node->gas_mass < almost_nothing && other->gas_mass < almost_nothing)){
            mass_flows.push_back(0);
            acceleration.push_back(0);
            velocity.push_back(0);
            continue;
        }

        // TODO I think this needs a distance component, possibly add the distance
        // from node's center of mass to each edge
        float distance = 1.0;
        float pressure_gradient = (other->pressure() - node->pressure())/distance;

        // Divide by density to get the presure contribution to flow acceleration
        float density = (node->density() + other->density())/2.0;
        float new_flow_acceleration = pressure_gradient/density;

        // Get the average acceleration in the period between the last update
        // and this one
        float current_flow_acceleration = (edge.acceleration + new_flow_acceleration)/2.0;

        // Calculate a velocity and flow
        float flow_velocity = current_flow_acceleration * delta_time + edge.velocity/2.0;
        float volumetric_flow = (flow_velocity + edge.velocity)/2.0 * edge.surface;

        // Convert to mass and store the values we have found
        float mass_flow = volumetric_flow * density * delta_time;
        mass_flows.push_back(mass_flow);
        acceleration.push_back(new_flow_acceleration);
        velocity.push_back(flow_velocity);
    }

    // Accumulate the total out flow
    float out_flow = 0;
    for(uint ii = 0; ii < node->edges.size(); ii++){
        if(mass_flows[ii] <= 0){
            out_flow += mass_flows[ii];
        }
    }

    // Get a scale so that the total out flow will at most
    float out_scale = std::min(1.0f, out_flow >= 0 ? 0 : node->gas_mass/-out_flow);

    // Apply the transaction
    for(uint ii = 0; ii < node->edges.size(); ii++){
        auto& edge = node->edges[ii];
        auto other = edge.other;
        float mass_flow;

        if(mass_flows[ii] <= 0)
            mass_flow = mass_flows[ii]*out_scale;
        else
            continue;

        if(mass_flow != mass_flow){
            std::cout << node->gas_mass << " " << mass_flow << " " << other->gas_mass << std::endl;
            exit(1);
        }

        mass_flow = std::max(mass_flow, -node->gas_mass);
        node->gas_mass += mass_flow;
        other->gas_mass -= mass_flow;

        Edge * other_edge = nullptr;
        for(auto& temp : other->edges){
            if(temp.other == node){
                other_edge = &temp;
                break;
            }
        }

        edge.acceleration = acceleration[ii] * out_scale;
        other_edge->acceleration = -acceleration[ii] * out_scale;
        edge.velocity = velocity[ii] * out_scale;
        other_edge->velocity = -velocity[ii] * out_scale;
    }
}

auto GasGraph::new_node() -> Node* {
    auto node = new Node;
    m_nodes.push_back(node);
    return node;
}

void GasGraph::remove_node(Node * node){
    // Disconnect the node from all others
    clear_edges(node);

    // Remove the node from the list
    auto iter = std::find(m_nodes.begin(), m_nodes.end(), node);

    if(iter != m_nodes.end()){
        std::iter_swap(iter, m_nodes.rbegin());
        m_nodes.pop_back();
    } else {
        // If we can't find the node issue a warning
        // TODO setup logging with levels
        debug << "Warning: Tried to free absent node?" << std::endl;
    }
}

void GasGraph::set_edge(Node* a, Node* b, float surface){
    a->edges.push_back({surface, b});
    b->edges.push_back({surface, a});
}

void GasGraph::clear_edges(Node* a){
    for(auto edge : a->edges){
        auto other = edge.other;
        for(uint ii = 0; ii < other->edges.size(); ii++){
            if(other->edges[ii].other == a){
                std::swap(other->edges[ii], other->edges.back());
                other->edges.pop_back();
                ii--;
            }
        }
    }
    a->edges.clear();
}
