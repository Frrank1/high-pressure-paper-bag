#include "gtest/gtest.h"

#include "Volume.hpp"

TEST(volume_tests, construction_volume_surface){
    {
        Volume v;
        ASSERT_EQ(v.volume(), 0);
        ASSERT_EQ(v.surface(), 0);
        ASSERT_FALSE(v);

        ASSERT_EQ(v.xmin(), 0);
        ASSERT_EQ(v.ymin(), 0);
        ASSERT_EQ(v.zmin(), 0);

        ASSERT_EQ(v.xmax(), 0);
        ASSERT_EQ(v.ymax(), 0);
        ASSERT_EQ(v.zmax(), 0);
    }

    {
        Volume v({0, 0, 0}, {1, 1, 1});
        ASSERT_EQ(v.volume(), 1);
        ASSERT_EQ(v.surface(), 6);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 0);
        ASSERT_EQ(v.ymin(), 0);
        ASSERT_EQ(v.zmin(), 0);

        ASSERT_EQ(v.xmax(), 1);
        ASSERT_EQ(v.ymax(), 1);
        ASSERT_EQ(v.zmax(), 1);
    }

    {
        Volume v({10, -10, 1}, {1, 1, 1});
        ASSERT_EQ(v.volume(), 1);
        ASSERT_EQ(v.surface(), 6);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 11);
        ASSERT_EQ(v.ymax(), -9);
        ASSERT_EQ(v.zmax(), 2);
    }

    {
        Volume v({10, -10, 1}, {0, 1, 1});
        ASSERT_EQ(v.volume(), 0);
        ASSERT_EQ(v.surface(), 2);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 10);
        ASSERT_EQ(v.ymax(), -9);
        ASSERT_EQ(v.zmax(), 2);
    }

    {
        Volume v({10, -10, 1}, {10, 10, 2});
        ASSERT_EQ(v.volume(), 200);
        ASSERT_EQ(v.surface(), 280);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 20);
        ASSERT_EQ(v.ymax(), 0);
        ASSERT_EQ(v.zmax(), 3);
    }
}

TEST(volume_tests, difference_operation){
    Volume a({0, 0, 0}, {10, 10, 10});

    ASSERT_EQ(surface({a}), 600);
    ASSERT_EQ(volume({a}), 1000);

    {
        Volume b({5, 0, 0}, {10, 10, 10});
        auto diff = a - b;

        ASSERT_EQ(surface(diff), 400);
        ASSERT_EQ(volume(diff), 500);
        ASSERT_EQ(diff.size(), 1);
        ASSERT_EQ(diff[0].offset, Point(0, 0, 0));
        ASSERT_EQ(diff[0].size, Size(5, 10, 10));
    }

    {
        Volume b({5, 5, 5}, {10, 10, 10});
        auto diff = a - b;

        ASSERT_EQ(surface(diff), 600);
        ASSERT_EQ(volume(diff), 1000 * 7 / 8);
    }

    {
        Volume b({5, 5, 0}, {10, 10, 10});
        auto diff = a - b;

        ASSERT_EQ(surface(diff), 100 + 100 + 100 + 100 + 100 * 2 * 3 / 4);
        ASSERT_EQ(volume(diff), 750);
    }
}

TEST(volume_tests, intersection_operation){
    Volume a({-10, -10, -10}, {12, 12, 12});
    Volume b({-1, -1, -1}, {10, 10, 10});

    Volume c = a & b;

    ASSERT_EQ(c.offset, Point(-1, -1, -1));
    ASSERT_EQ(c.size, Size(3, 3, 3));
}

TEST(volume_tests, bounding_operation){
    Volume a({-10, -10, -10}, {10, 10, 10});
    Volume b({9, 9, 9}, {1, 1, 1});

    Volume c = a | b;

    ASSERT_EQ(c.offset, Point(-10, -10, -10));
    ASSERT_EQ(c.size, Size(21, 21, 21));
}

TEST(volume_tests, overlap_operation){
    Volume a({0, 0, 0}, {1, 1, 1});
    Volume b({0, 0, 0}, {2, 2, 2});
    Volume c({1, 0, 0}, {1, 1, 1});

    ASSERT_TRUE(a.overlap(b));
    ASSERT_TRUE(b.overlap(a));

    ASSERT_FALSE(a.overlap(c));
    ASSERT_FALSE(c.overlap(a));
}

TEST(volume_tests, adjacent_operation){
    Volume a({0, 0, 0}, {1, 1, 1});
    Volume b({10, 10, 10}, {1, 1, 1});
    Volume c({1, 0, 0}, {9, 10, 10});

    ASSERT_FALSE(a.adjacent(b));
    ASSERT_FALSE(b.adjacent(a));

    ASSERT_TRUE(a.adjacent(c));
    ASSERT_TRUE(c.adjacent(a));

    // ASSERT_FALSE(a.adjacent(b));
    // ASSERT_FALSE(b.adjacent(a));
}

TEST(volume_tests, gap_operation){
    Volume a({0, 0, 0}, {1, 1, 1});
    Volume b({10, 10, 10}, {1, 1, 1});
    Volume c({1, 0, 0}, {9, 10, 10});

    ASSERT_EQ(a.gap(b, 0), 9);
    ASSERT_EQ(a.gap(b, 1), 9);
    ASSERT_EQ(a.gap(b, 2), 9);

    // ASSERT_FALSE(a.adjacent(b));
    // ASSERT_FALSE(b.adjacent(a));
}

// TEST(volume_tests, contact_operation){
// }

// TEST(volume_tests, contains_operation){
// }

TEST(volume_tests, set_difference_operation){
    std::vector<Volume> a {
        Volume({0, 0, 0}, {2, 10, 10}),
        Volume({2, 0, 0}, {2, 10, 10}),
        Volume({4, 0, 0}, {4, 10, 10}),
        Volume({8, 0, 0}, {2, 10, 10})
    };

    ASSERT_EQ(surface(a), 600);
    ASSERT_EQ(volume(a), 1000);

    {
        Volume b({5, 0, 0}, {10, 10, 10});
        auto diff = a - b;

        ASSERT_EQ(surface(diff), 400);
        ASSERT_EQ(volume(diff), 500);
    }

    {
        Volume b({5, 5, 5}, {10, 10, 10});
        auto diff = a - b;
        ASSERT_EQ(surface(diff), 600);
        ASSERT_EQ(volume(diff), 1000 * 7 / 8);
    }

    {
        Volume b({5, 5, 0}, {10, 10, 10});
        auto diff = a - b;

        ASSERT_EQ(surface(diff), 100 + 100 + 100 + 100 + 100 * 2 * 3 / 4);
        ASSERT_EQ(volume(diff), 750);
    }
}

// TEST(volume_tests, set_surface_operation){
// }

// TEST(volume_tests, set_volume_operation){
// }

// TEST(volume_tests, set_contact_operation){
// }

// TEST(volume_tests, set_compact_operation){
// }

int main(int argc, char *argv[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
