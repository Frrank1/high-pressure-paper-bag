#include "GasGraph.hpp"

#include <unordered_set>
#include <cassert>

//
//
//

float GasGraph::Node::density() const {
    return gas / volume;
}

//
//
//

GasGraph::GasGraph(){}
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

        // Quick and dirty
        auto copy = remaining;
        for(auto node : copy) {
            if(adjacent.count(node)) continue;
            remaining.erase(node);

            step_node(node, delta);

            for(auto edge : node->edges)
                adjacent.insert(edge.other);
        }
    }
}

void GasGraph::step_node(Node* node, float /*delta*/){
    if(node->volume == 0) return;
    for(auto edge : node->edges){
        auto other = edge.other;
        if(other->volume == 0) continue;
        // A non-physical calculation of flow based on surface area relative
        // to the total surface area.
        float ratio = edge.surface / node->surface;

        // Based the highest possible shared surface being half at the limit
        ratio /= 1.0/2.0;
        assert(0 <= ratio and ratio <= 1.0);

        // Boost it quadratically on accord of nothing in particular
        // Air is slippery I guess?
        ratio = (1 - (1 - ratio) * (1 - ratio));

        // Make sure there is at least SOMETHING coming out of this calculation
        // ratio = ratio * 0.95 + 0.05;

        // Calculate the equalibrium values
        float equal_density = (other->density() + node->density())/2.0;
        float here_delta = (equal_density * node->volume) - node->gas;
        float other_delta = -((equal_density * other->volume) - other->gas);

        // Get whichever is willing to change less
        float change = abs_min(here_delta, other_delta);

        // TODO add in effect of time delta
        node->gas += change * ratio * 0.5;
        other->gas -= change * ratio * 0.5;
    }
}

auto GasGraph::new_node() -> Node* {
    auto node = new Node;
    m_nodes.push_back(node);
    return node;
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
