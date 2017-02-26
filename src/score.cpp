/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "score.hpp"
#include "Volume.hpp"
#include "Cluster.hpp"

#include <fstream>
#include <tuple>
#include <algorithm>
#include <unordered_map>

//
// Helper functions that are limited to this module.
//
namespace {
    // TODO currently using a null stream to eat logging info.
    auto debug = std::ofstream();

    // Give the score for splitting shape on the plain defined by seam and
    // (normal) axis parameters.
    template <int axis> float cut_cost(const Cluster& shape, Volume bounds, int seam){
        // The axis on which the plane lies will be the other two since 'axis'
        // is normal to it
        const int a1 = (axis + 1) % 3;
        const int a2 = (axis + 2) % 3;

        // Modify the bouding box to be the cut plane
        auto cut = bounds;
        cut.offset[axis] = seam;
        cut.size[axis] = 0;

        // find all the faces being crossed by the cut plane
        float cost = 0;
        for(auto part : shape){
            // If the gap is less than zero the cut plane is inside the part
            // (zero would mean it was just outside)
            auto gap = part.gap<axis>(cut);
            if(gap < 0){
                cost += part.size[a1] * part.size[a2];
            }
        }

        return cost;
    }

    // Consider cutting the shape orthegonal to the given axis.
    //
    // The return is information about the cheapest cut
    // (its cost, its score, and where it is on the axis)
    // The cost is the "how much we don't want to do this cut" value.
    // the score is the cost combined with a measure of possible gain from the cut
    template <int axis> std::tuple<float, float, int>
    score_axis(const Cluster& shape){
        // Define the axis this method will be cutting on
        debug << "axis " << axis << std::endl;
        const int a1 = (axis + 1) % 3;
        const int a2 = (axis + 2) % 3;

        // Get the information of the shape so they arn't always being recalculated
        const auto bounds = shape.bounds();
        const auto total_volume = shape.volume();

        // Determine the spots where there are edges of volumes
        // accumulate the ammount of surface area where two volumes are
        // in contact along the plane
        std::unordered_map<int, float> cut_points;
        for(uint ii = 0; ii < shape.size(); ii++){
            auto part = shape[ii];
            // Check the top and bottom of this shape along the axis
            for(int index : {part.offset[axis], part.offset[axis] + int(part.size[axis])}){
                // Skip this index if it lies along the edge of the bounding box
                if(bounds.offset[axis] < index and index + 1 < bounds.offset[axis] + int(bounds.size[axis])){
                    // We are not along the edge of the bounding box, so accumulate
                    // the contact area between volumes in this shape, along this
                    // plane
                    float contact = 0;
                    for(uint jj = 0; jj < shape.size(); jj++){
                        if(ii == jj) continue;
                        // See if the other shapes make contact with this part
                        auto other = shape[jj];
                        if(part.gap<axis>(other) == 0){
                            // Add in contact, but only if it lies on this
                            // plane (defined here by orthegonal 'axis' and
                            // the offset 'index')
                            contact += part.contact<axis>(other, index);
                        }
                    }
                    // We will see this again when we go by this again, so
                    // half it (TODO don't go through twice)
                    cut_points[index] += contact/2.0;
                }
            }
        }

        // If there are no cut points this shape is not going to be split
        if(cut_points.empty()){
            return std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), 0);
        }

        debug << "Cut points ";
        for(auto pair : cut_points) debug << pair.first << ":" << pair.second << " ";
        debug << std::endl;

        // We are going to search through the cut points and find the
        // one with the best (lowest) price and score
        int cut_point = 0;
        float cut_price = std::numeric_limits<float>::infinity();
        float cut_score = std::numeric_limits<float>::infinity();

        float bound_cut = bounds.size[a1] * bounds.size[a2];
        for(auto pair : cut_points){
            // Get the price for cutting at this point
            auto point_price = (cut_cost<axis>(shape, bounds, pair.first) + pair.second)/bound_cut;
            // TODO Maybe one of these
            //point_price = 1 - (1 - point_price) * (1 -point_price);
            //point_price = point_price / (1-point_price);

            debug << "=== Cutting at " << pair.first << " for " << point_price << std::endl;

            // Define bounding volumes for the two halves created by splitting
            // on this cut point
            auto half_one = bounds;
            half_one.size[axis] = pair.first - half_one.offset[axis];

            auto half_two = bounds;
            half_two.size[axis] = bounds.size[axis] - half_one.size[axis];
            half_two.offset[axis] = pair.first;

            // Calculate the percentage of the volume on either side of the
            // cut point
            auto volume_one = (shape & half_one).volume()/total_volume;
            auto volume_two = (shape & half_two).volume()/total_volume;

            // If we can split a shape in half we have a perfect cut
            debug << volume_one << " " << volume_two << std::endl;
            float quality = std::min(volume_one, volume_two)/std::max(volume_one, volume_two);
            // quality = quality / (1 - quality);
            // quality = sigmoid(quality);

            // The over all score is a trade off betwenn the price of the cut
            // and the quality of the cut
            float point_score = point_price/quality;
            debug << quality << " " << point_score << std::endl;

            // Take the lowest price, if that isn't improved take the lowest
            // score
            if(point_price < cut_price){
                cut_price = point_price;
                cut_point = pair.first;
                cut_score = point_score;
            } else if(cut_price == point_price and point_score < cut_score){
                cut_price = point_price;
                cut_point = pair.first;
                cut_score = point_score;
            }
        }


        debug << " == " << cut_score << std::endl;

        return std::make_tuple(cut_price, cut_score, cut_point);
    }

    template std::tuple<float, float, int> score_axis<0>(const Cluster& shape);
}

// Find a split and give it a score
Split score(const std::vector<Volume>& shape){
    debug << "-------------------" << std::endl;

    float cost1, cost2, cost3;
    float score1, score2, score3;
    int index[3];

    // Find what the suggested split on each axis is
    std::tie(cost1, score1, index[0]) = score_axis<0>(shape);
    std::tie(cost2, score2, index[1]) = score_axis<1>(shape);
    std::tie(cost3, score3, index[2]) = score_axis<2>(shape);

    // See which axis offers the best cost
    float cost = std::min({cost1, cost2, cost3});
    std::vector<int> selected_axis;
    std::vector<float> score;
    if(cost1 == cost){ selected_axis.push_back(0); score.push_back(score1); }
    if(cost2 == cost){ selected_axis.push_back(1); score.push_back(score2); }
    if(cost3 == cost){ selected_axis.push_back(2); score.push_back(score3); }

    // If one axis is clearly better, use it
    if(score.size() == 1){
        return Split{score[0], index[selected_axis[0]], selected_axis[0]};
    }

    // If there is a tie break it by score
    float final_score = *std::min_element(score.begin(), score.end());
    selected_axis.clear();
    if(score1 == final_score){ selected_axis.push_back(0); }
    if(score2 == final_score){ selected_axis.push_back(1); }
    if(score3 == final_score){ selected_axis.push_back(2); }
    return Split{final_score, index[selected_axis[0]], selected_axis[0]};
}
