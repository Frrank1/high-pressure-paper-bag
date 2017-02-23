/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
/**
 * An R-Tree template for spacial searching.
 *
 * Fairly simple implementation without any special tricks.
 * Has not been tuned or profiled.
 */
#ifndef HPPB_SRC_RTREE_HPP
#define HPPB_SRC_RTREE_HPP

#include <vector>
#include <unordered_map>

#include "Volume.hpp"
#include "Cluster.hpp"

#define RTREE_TEMPLATE template <class Type, int Dimensions, int MinChildren, int MaxChildren>
#define RTREE_CLASS RTree<Type, Dimensions, MinChildren, MaxChildren>
#define RTREENODE_CLASS RTreeNode<Type, Dimensions, MinChildren, MaxChildren>

RTREE_TEMPLATE
class RTreeNode;

template <class Type, int Dimensions=3, int MinChildren=4, int MaxChildren=32>
class RTree {
public:
    typedef RTreeNode<Type, Dimensions, MinChildren, MaxChildren> NodeType;

public:
    RTree();
    ~RTree();

public:
    void insert(Type, Volume);
    void move(Type, Volume);
    void remove(Type);

public:
    std::vector<Type> intersecting(Volume) const;
    std::vector<Type> inside(Volume) const;
    Volume find(Type) const;

protected:
    void split_root();

protected:
    NodeType * m_root = nullptr;
    std::unordered_map<Type, Volume> m_locations;
};

//
//  Node declaration
//

RTREE_TEMPLATE
class RTreeNode {
public:
    typedef RTreeNode<Type, Dimensions, MinChildren, MaxChildren> NodeType;
    friend RTREE_CLASS;

public:
    RTreeNode();
    RTreeNode(const std::vector<NodeType*>&);
    RTreeNode(const std::vector<std::pair<Volume, Type>>&);
    ~RTreeNode();

public:
    void insert(Type, Volume);
    // Returns whether the bounding box has changed
    bool remove(Type, Volume);

public:
    void intersecting(Volume, std::vector<Type>&) const;
    void inside(Volume, std::vector<Type>&) const;
    void all(std::vector<std::pair<Volume, Type>>&) const;

public:
    uint size() const;
    // TODO re-implement to be in constant time
    uint real_size() const;

protected:
    int64_t expansion(Volume) const;
    NodeType* split_self();
    void absorb_child(NodeType*);
    void split_child(NodeType*);
    void update_bounds();

protected:
    const bool m_internal;
    std::vector<std::pair<Volume, Type>> m_data;
    std::vector<NodeType*> m_children;
    Volume m_bounds;
};

//
//  RTree implementation
//

RTREE_TEMPLATE RTREE_CLASS::RTree(){
    m_root = new NodeType;
}

RTREE_TEMPLATE RTREE_CLASS::~RTree(){
    delete m_root;
}

RTREE_TEMPLATE
void RTREE_CLASS::insert(Type value, Volume bounds){
    m_root->insert(value, bounds);
    m_locations[value] = bounds;
    split_root();
}

RTREE_TEMPLATE
void RTREE_CLASS::move(Type value, Volume bounds){
    m_root->remove(value, m_locations[value]);
    m_root->insert(value, bounds);
    m_locations[value] = bounds;
}

RTREE_TEMPLATE
void RTREE_CLASS::remove(Type value){
    if(!m_locations.count(value)) return;
    m_root->remove(value, m_locations[value]);
    m_locations.erase(value);
}

RTREE_TEMPLATE
std::vector<Type> RTREE_CLASS::intersecting(Volume bounds) const {
    std::vector<Type> output;
    m_root->intersecting(bounds, output);
    return output;
}

RTREE_TEMPLATE
std::vector<Type> RTREE_CLASS::inside(Volume bounds) const {
    std::vector<Type> output;
    m_root->inside(bounds, output);
    return output;
}

RTREE_TEMPLATE
Volume RTREE_CLASS::find(Type value) const {
    return m_locations[value];
}

RTREE_TEMPLATE
void RTREE_CLASS::split_root(){
    if(m_root->size() > MaxChildren){
        auto other = m_root->split_self();
        m_root = new NodeType(std::vector<NodeType*>{m_root, other});
    }
}

//
//  Node implementation
//

RTREE_TEMPLATE
RTREENODE_CLASS::RTreeNode()
:   m_internal(false)
{
}

RTREE_TEMPLATE
RTREENODE_CLASS::RTreeNode(const std::vector<NodeType*>& data)
:   m_internal(true)
,   m_children(data)
{
    update_bounds();
}

RTREE_TEMPLATE
RTREENODE_CLASS::RTreeNode(const std::vector<std::pair<Volume, Type>>& data)
:   m_internal(false)
,   m_data(data)
{
    update_bounds();
}


RTREE_TEMPLATE
RTREENODE_CLASS::~RTreeNode(){
    if(m_internal){
        for(auto child : m_children)
            delete child;
    }
}

RTREE_TEMPLATE
void RTREENODE_CLASS::insert(Type value, Volume bounds){
    if(m_internal){
        // See which of the child nodes will expand the least to include
        // the new value
        NodeType* candidate = m_children.front();
        auto expansion = candidate->expansion(bounds);

        for(auto node : m_children){
            auto node_expansion = node->expansion(bounds);
            if(node_expansion < expansion){
                expansion = node_expansion;
                candidate = node;
            } else if (node_expansion == expansion){
                // Break ties in expansion with occupancy
                if(node->size() < candidate->size())
                    candidate = node;
            }
        }

        // Put the new data in the selected child
        candidate->insert(value, bounds);
        m_bounds = m_bounds | candidate->m_bounds;
        split_child(candidate);

    } else {
        // Add to the leves in this node and increase the bounding box
        m_data.emplace_back(bounds, value);
        if(m_data.size() == 1)
            m_bounds = bounds;
        else
            m_bounds = m_bounds | bounds;
    }
}

RTREE_TEMPLATE
bool RTREENODE_CLASS::remove(Type value, Volume bounds){
    auto starting_bounds = m_bounds;
    bool update = false;

    if(m_internal){
        std::vector<NodeType*> called_children;
        for(auto child : m_children){
            if(child->m_bounds.overlap(bounds)){
                update |= child->remove(value, bounds);
                called_children.push_back(child);
            }
        }
        for(auto child : called_children)
            absorb_child(child);
    } else {
        for(uint ii = 0; ii < m_data.size(); ii++){
            if(m_data[ii].second == value){
                update = true;
                std::swap(m_data[ii], m_data.back());
                m_data.pop_back();
                break;
            }
        }
    }

    if(update) update_bounds();
    return m_bounds != starting_bounds;
}

RTREE_TEMPLATE
void RTREENODE_CLASS::intersecting(Volume bounds, std::vector<Type>& output) const{
    if(m_internal){
        for(auto child : m_children){
            if(child->m_bounds.overlap(bounds)){
                child->intersecting(bounds, output);
            }
        }
    } else {
        for(auto child : m_data){
            if(child.first.overlap(bounds)){
                output.push_back(child.second);
            }
        }
    }
}

RTREE_TEMPLATE
void RTREENODE_CLASS::inside(Volume bounds, std::vector<Type>& output) const {
    if(m_internal){
        for(auto child : m_children){
            if(child->m_bounds.overlap(bounds)){
                child->inside(bounds, output);
            }
        }
    } else {
        for(auto child : m_data){
            if(bounds.contains(child.first)){
                output.push_back(child.second);
            }
        }
    }
}

RTREE_TEMPLATE
void RTREENODE_CLASS::all(std::vector<std::pair<Volume, Type>>& output) const {
    if(m_internal){
        for(auto child : m_children)
            child->all(output);
    } else {
        output.insert(output.end(), m_data.begin(), m_data.end());
    }
}

RTREE_TEMPLATE
uint RTREENODE_CLASS::size() const {
    if(m_internal){
        return m_children.size();
    } else {
        return m_data.size();
    }
}

RTREE_TEMPLATE
uint RTREENODE_CLASS::real_size() const {
    if(m_internal){
        uint value = 0;
        for(auto child : m_children)
            value += child->real_size();
        return value;
    } else {
        return m_data.size();
    }
}

RTREE_TEMPLATE
int64_t RTREENODE_CLASS::expansion(Volume bounds) const {
    return ((bounds | m_bounds) - m_bounds).volume();
}

RTREE_TEMPLATE
auto RTREENODE_CLASS::split_self() -> NodeType* {
    // Which axis are we splitting on
    int axis = 0;
    uint axis_size = m_bounds.size[0];
    for(int ii = 1; ii < Dimensions; ii++){
        if(axis_size < m_bounds.size[ii]){
            axis = ii;
            axis_size = m_bounds.size[ii];
        }
    }

    if(m_internal){
        // Find a median value on that axis
        std::vector<float> centers;
        for(auto child : m_children) centers.push_back(child->m_bounds.center(axis));
        float median = centers[centers.size()/2];

        // We are going to divide the children into groups
        decltype(m_children) left_group, right_group;
        for(auto child : m_children){
            if(child->m_bounds.center(axis) < median){
                left_group.push_back(child);
            } else {
                right_group.push_back(child);
            }
        }

        m_children = left_group;
        update_bounds();
        return new NodeType(right_group);

    } else {
        // Find a median value on that axis
        std::vector<float> centers;
        for(auto child : m_data) centers.push_back(child.first.center(axis));
        float median = centers[centers.size()/2];

        // We are going to divide the children into groups
        decltype(m_data) left_group, right_group;
        for(auto child : m_data){
            if(child.first.center(axis) < median){
                left_group.push_back(child);
            } else {
                right_group.push_back(child);
            }
        }

        m_data = left_group;
        update_bounds();
        return new NodeType(right_group);
    }
}

RTREE_TEMPLATE
void RTREENODE_CLASS::absorb_child(NodeType* child){
    // if a branch doesn't have enough values
    if(child->real_size() < MinChildren){
        // Take the data
        decltype(m_data) data;
        child->all(data);

        // remove the child
        {
            auto iter = std::find(m_children.begin(), m_children.end(), child);
            std::iter_swap(iter, m_children.rbegin());
            m_children.pop_back();
            delete child;
            child = nullptr;
        }

        // Re-insert the data
        for(auto item : data){
            insert(item.second, item.first);
        }
    }
    // TODO add someting for rebalancing when internal nodes get sparse
    // or list like.
}

RTREE_TEMPLATE
void RTREENODE_CLASS::split_child(NodeType* child){
    if(child->size() > MaxChildren){
        auto new_child = child->split_self();
        m_children.push_back(new_child);
        update_bounds();
    }
}

RTREE_TEMPLATE
void RTREENODE_CLASS::update_bounds(){
    if(m_internal){
        m_bounds = m_children.front()->m_bounds;
        for(uint ii = 1; ii < m_children.size(); ii++){
            m_bounds = m_bounds | m_children[ii]->m_bounds;
        }
    } else {
        m_bounds = m_data.front().first;
        for(uint ii = 1; ii < m_data.size(); ii++){
            m_bounds = m_bounds | m_data[ii].first;
        }
    }
}

#undef RTREE_TEMPLATE
#undef RTREE_CLASS
#endif
