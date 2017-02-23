/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
/**
 * A irregular region of space represented by a cluster of volumes.
 */
#ifndef HPPB_SRC_CLUSTER_HPP
#define HPPB_SRC_CLUSTER_HPP

#include <iterator>
#include <vector>
#include <initializer_list>

#include "Volume.hpp"

//
//  In all operations where an assumption that volumes in a set don't
//  overlap makes sense, that assumption is in place.
//


class Cluster {
public:
    Cluster();
    Cluster(std::initializer_list<Volume>);
    Cluster(const std::vector<Volume>&);
    Cluster(const std::vector<Volume>&&);

public:
    Cluster(const Cluster&);
    Cluster& operator = (const Cluster&);

public:
    Cluster(Cluster&&);
    Cluster& operator = (Cluster&&);

protected:
    void init();

public:
    // Clusters of volumes are iterable
    struct const_iterator : public std::iterator<std::input_iterator_tag, Volume> {
        explicit const_iterator(const Cluster&, int);
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const_iterator other);
        bool operator!=(const_iterator other);
        value_type operator*() const;

    private:
        const Cluster& source;
        int index;
    };

    const_iterator begin() const;
    const_iterator end() const;
    Volume operator[](uint) const;

public:

    bool empty() const;
    size_t size() const;
    bool operator == (const Cluster&) const;
    bool operator != (const Cluster&) const;
    const std::vector<Volume>& parts() const;

    // Calculate the external surface area of a set of volumes
    float surface() const;
    Volume bounds() const;
    // Calculate the total volume of a set volumes
    float volume() const;


public:
    void add(Volume);

    // Try to remove redundancy or merge volumes where possible
    void compact();

public:
    // Return all parts of space in the given set that do not overlap with the
    // volume given
    Cluster operator - (Volume) const;
    Cluster operator & (Volume) const;

    // Test if a volume is adjacent to any item of the set
    bool adjacent(Volume) const;

    // Calculate the surface area in contact between two sets of volumes
    float contact(const Cluster&) const;
    bool overlap(Volume) const;

    // Return the set of volumes broken into connected sections
    std::vector<Cluster> connected_components() const;

protected:
    std::vector<Volume> m_volumes;
    float m_surface = 0;
    float m_volume = 0;
    Volume m_bounds;
};

std::ostream& operator << (std::ostream&, const Cluster&);

#endif
