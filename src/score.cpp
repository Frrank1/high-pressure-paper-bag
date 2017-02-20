/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2017 Adam Douglass
 */
#include "score.hpp"
#include "Volume.hpp"

#include <fstream>
#include <tuple>
#include <algorithm>
#include <unordered_map>

namespace {
    auto debug = std::ofstream();

    template <int axis> float cut_cost(const std::vector<Volume>& shape, int seam){
        const int a1 = (axis + 1) % 3;
        const int a2 = (axis + 2) % 3;
        auto bounds = ::bounds(shape);

        auto cut = bounds;
        cut.offset[axis] = seam;
        cut.size[axis] = 0;

        float cost = 0;
        for(auto part : shape){
            auto gap = part.gap<axis>(cut);
            if(gap < 0){
                cost += part.size[a1] * part.size[a2];
            }
        }

        return cost;
    }

    template <int axis> std::tuple<float, float, int>
    score_axis(const std::vector<Volume>& shape){
        debug << "axis " << axis << std::endl;
        const int a1 = (axis + 1) % 3;
        const int a2 = (axis + 2) % 3;
        auto bounds = ::bounds(shape);
        auto total_volume = volume(shape);

        // Determine the spots where there are edges
        std::unordered_map<int, float> cut_points;
        for(uint ii = 0; ii < shape.size(); ii++){
            auto part = shape[ii];
            for(int index : {part.offset[axis], part.offset[axis] + int(part.size[axis])}){
                if(bounds.offset[axis] < index and index + 1 < bounds.offset[axis] + int(bounds.size[axis])){
                    float contact = 0;

                    for(uint jj = 0; jj < shape.size(); jj++){
                        if(ii == jj) continue;
                        auto other = shape[jj];

                        if(part.gap<axis>(other) == 0){
                            contact += part.contact<axis>(other, index);
                        }
                    }
                    cut_points[index] += contact/2.0;
                }
            }
        }

        if(cut_points.empty()){
            return std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), 0);
        }

        debug << "Cut points ";
        for(auto pair : cut_points) debug << pair.first << ":" << pair.second << " ";
        debug << std::endl;

        int cut_point = 0;
        float cut_price = std::numeric_limits<float>::infinity();
        float cut_score = std::numeric_limits<float>::infinity();

        float bound_cut = bounds.size[a1] * bounds.size[a2];
        for(auto pair : cut_points){
            auto point_price = (cut_cost<axis>(shape, pair.first) + pair.second)/bound_cut;
            // TODO Maybe one of these
            //point_price = 1 - (1 - point_price) * (1 -point_price);
            //point_price = point_price / (1-point_price);

            debug << "=== Cutting at " << pair.first << " for " << point_price << std::endl;

            auto half_one = bounds;
            half_one.size[axis] = pair.first - half_one.offset[axis];

            auto half_two = bounds;
            half_two.size[axis] = bounds.size[axis] - half_one.size[axis];
            half_two.offset[axis] = pair.first;

            auto volume_one = volume(shape & half_one)/total_volume;
            auto volume_two = volume(shape & half_two)/total_volume;

            debug << volume_one << " " << volume_two << std::endl;
            float gain = std::min(volume_one, volume_two)/std::max(volume_one, volume_two);
            // gain = gain / (1 - gain);
            // gain = sigmoid(gain);

            float point_score = point_price/gain;
            debug << gain << " " << point_score << std::endl;

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
}

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
