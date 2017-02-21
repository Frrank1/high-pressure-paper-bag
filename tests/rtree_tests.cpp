#include <random>

#include "gtest/gtest.h"
#include "Volume.hpp"
#include "RTree.hpp"

std::vector<int> brute_force(const std::vector<Volume>& volumes, Volume bounds){
    std::vector<int> out;
    for(uint ii = 0; ii < volumes.size(); ii++){
        if(volumes[ii].overlap(bounds)){
            out.push_back(ii);
        }
    }
    return out;
}

TEST(volume_tests, insert_against_brute_force){
    // Get a long sequence of numbers
    std::mt19937_64 prng(10);
    std::uniform_int_distribution<> size_distribution(1, 100);
    std::uniform_int_distribution<> offset_distribution(0, 1000);

    // Construct an RTree and load it with volumes
    RTree<int> tree;
    std::vector<Volume> items;
    for(int ii = 0; ii < 10000; ii++){
        Volume current(
            {offset_distribution(prng), offset_distribution(prng), offset_distribution(prng)},
            {size_distribution(prng), size_distribution(prng), size_distribution(prng)}
        );
        items.push_back(current);
        tree.insert(ii, current);
    }

    // Perform some queries
    for(int ii = 0; ii < 10000; ii++){
        Volume search(
            {offset_distribution(prng), offset_distribution(prng), offset_distribution(prng)},
            {size_distribution(prng), size_distribution(prng), size_distribution(prng)}
        );

        auto target = brute_force(items, search);
        auto result = tree.intersecting(search);
        ASSERT_EQ(target.size(), result.size());
        std::sort(result.begin(), result.end());
        ASSERT_EQ(target, result);
    }
}

TEST(volume_tests, remove_against_brute_force){
    // Get a long sequence of numbers
    std::mt19937_64 prng(10);
    std::uniform_int_distribution<> size_distribution(1, 100);
    std::uniform_int_distribution<> offset_distribution(0, 1000);

    // Construct an RTree and load it with volumes
    RTree<int> tree;
    std::vector<Volume> items;
    for(int ii = 0; ii < 10000; ii++){
        Volume current(
            {offset_distribution(prng), offset_distribution(prng), offset_distribution(prng)},
            {size_distribution(prng), size_distribution(prng), size_distribution(prng)}
        );
        items.push_back(current);
        tree.insert(ii, current);
    }

    // Remove half the items
    size_t start = items.size();
    while(items.size() > start/2){
        tree.remove(items.size() - 1);
        items.pop_back();
    }

    // Perform some queries
    for(int ii = 0; ii < 10000; ii++){
        Volume search(
            {offset_distribution(prng), offset_distribution(prng), offset_distribution(prng)},
            {size_distribution(prng), size_distribution(prng), size_distribution(prng)}
        );

        auto target = brute_force(items, search);
        auto result = tree.intersecting(search);
        ASSERT_EQ(target.size(), result.size());
        std::sort(result.begin(), result.end());
        ASSERT_EQ(target, result);
    }
}

int main(int argc, char *argv[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
