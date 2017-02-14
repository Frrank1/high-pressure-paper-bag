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

        ASSERT_EQ(v.xmax(), -1);
        ASSERT_EQ(v.ymax(), -1);
        ASSERT_EQ(v.zmax(), -1);
    }

    {
        Volume v({0, 0, 0}, {1, 1, 1});
        ASSERT_EQ(v.volume(), 1);
        ASSERT_EQ(v.surface(), 6);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 0);
        ASSERT_EQ(v.ymin(), 0);
        ASSERT_EQ(v.zmin(), 0);

        ASSERT_EQ(v.xmax(), 0);
        ASSERT_EQ(v.ymax(), 0);
        ASSERT_EQ(v.zmax(), 0);
    }

    {
        Volume v({10, -10, 1}, {1, 1, 1});
        ASSERT_EQ(v.volume(), 1);
        ASSERT_EQ(v.surface(), 6);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 10);
        ASSERT_EQ(v.ymax(), -10);
        ASSERT_EQ(v.zmax(), 1);
    }

    {
        Volume v({10, -10, 1}, {0, 1, 1});
        ASSERT_EQ(v.volume(), 0);
        ASSERT_EQ(v.surface(), 2);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 9);
        ASSERT_EQ(v.ymax(), -10);
        ASSERT_EQ(v.zmax(), 1);
    }

    {
        Volume v({10, -10, 1}, {10, 10, 2});
        ASSERT_EQ(v.volume(), 200);
        ASSERT_EQ(v.surface(), 280);
        ASSERT_TRUE(v);

        ASSERT_EQ(v.xmin(), 10);
        ASSERT_EQ(v.ymin(), -10);
        ASSERT_EQ(v.zmin(), 1);

        ASSERT_EQ(v.xmax(), 19);
        ASSERT_EQ(v.ymax(), -1);
        ASSERT_EQ(v.zmax(), 2);
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
        Volume b({0, 0, 0}, {5, 5, 5});
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

    {
        Volume a({0, 0, 0}, {3, 2, 2});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 4 * 2 + 6 * 4 + 2);

    }
    {
        Volume a({0, 0, 0}, {3, 3, 2});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);
    }

    {
        Volume a({0, 0, 0}, {3, 2, 3});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);

    }

    {
        Volume a({0, 0, 0}, {2, 3, 3});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);

    }

    {
        Volume a({0, 0, 1}, {3, 3, 2});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);

    }

    {
        Volume a({0, 1, 0}, {3, 2, 3});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);

    }

    {
        Volume a({1, 0, 0}, {2, 3, 3});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 + 6 * 4 + 9 + 4);

    }

    {
        Volume a({0, 0, 0}, {3, 3, 3});
        Volume b({1, 1, 1}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), 9 * 6 + 6);
    }

    {
        Volume b({5, 5, 5}, {1, 1, 1});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), a.surface() + b.surface());
    }
    {
        Volume b({1, 1, 1}, {8, 8, 8});
        auto diff = a - b;

        EXPECT_EQ(volume(diff), a.volume() - b.volume());
        EXPECT_EQ(surface(diff), a.surface() + b.surface());
    }

    {
        Volume a({10, 0, 0}, {3, 3, 3});
        Volume b({20, 0, 0}, {3, 1, 1});
        auto diff = a - b;

        ASSERT_EQ(diff.size(), 1);
        EXPECT_EQ(diff[0].offset, a.offset);
        EXPECT_EQ(diff[0].size, a.size);
    }

    {
        Volume a({10, 0, 0}, {3, 3, 3});
        Volume b({1, 0, 0}, {3, 1, 1});
        auto diff = a - b;

        ASSERT_EQ(diff.size(), 1);
        EXPECT_EQ(diff[0].offset, a.offset);
        EXPECT_EQ(diff[0].size, a.size);
    }

    {
        Volume a({0, 10, 0}, {3, 3, 3});
        Volume b({1, 0, 0}, {3, 1, 1});
        auto diff = a - b;

        ASSERT_EQ(diff.size(), 1);
        EXPECT_EQ(diff[0].offset, a.offset);
        EXPECT_EQ(diff[0].size, a.size);
    }

    {
        Volume a({0, 0, 10}, {3, 3, 3});
        Volume b({1, 0, 0}, {3, 1, 1});
        auto diff = a - b;

        ASSERT_EQ(diff.size(), 1);
        EXPECT_EQ(diff[0].offset, a.offset);
        EXPECT_EQ(diff[0].size, a.size);
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

    EXPECT_EQ(c.xmin(), -10);
    EXPECT_EQ(c.ymin(), -10);
    EXPECT_EQ(c.zmin(), -10);

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

TEST(volume_tests, contact_operation){
    {
        Volume a({1, 1, 1}, {3, 3, 3});

        Volume b({4, 1, 1}, {3, 3, 3});
        ASSERT_EQ(a.contact(b), 9);
        ASSERT_EQ(b.contact(a), 9);

        Volume c({4, 2, 2}, {3, 3, 3});
        ASSERT_EQ(a.contact(c), 4);
        ASSERT_EQ(c.contact(a), 4);

        Volume d({4, 3, 3}, {3, 3, 3});
        ASSERT_EQ(a.contact(d), 1);
        ASSERT_EQ(d.contact(a), 1);
    }

    {
        Volume a({0, 0, 0}, {10, 1, 10});
        Volume b({0, 1, 0}, {1, 10, 10});

        ASSERT_EQ(a.contact(b), 10);
        ASSERT_EQ(b.contact(a), 10);
    }

    {
        Volume a({0, 0, 0}, {1, 1, 1});

        Volume b({1, 1, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(b), 0);
        ASSERT_EQ(b.contact(a), 0);

        Volume c({1, 0, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(c), 1);
        ASSERT_EQ(c.contact(a), 1);

        Volume d({2, 0, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(d), 0);
        ASSERT_EQ(d.contact(a), 0);

        Volume e({1, 1, 1}, {1, 1, 1});
        ASSERT_EQ(a.contact(e), 0);
        ASSERT_EQ(e.contact(a), 0);

        Volume f({-1, 0, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(f), 1);
        ASSERT_EQ(f.contact(a), 1);

        Volume g({0, 1, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(g), 1);
        ASSERT_EQ(g.contact(a), 1);

        Volume h({2, 1, 0}, {1, 1, 1});
        ASSERT_EQ(a.contact(h), 0);
        ASSERT_EQ(h.contact(a), 0);
    }

    {
        {
            // <(0, 0) (0, 2) (0, 1)> 22 6
            Volume a({0, 0, 0}, {1, 3, 2});
            // <(2, 2) (0, 2) (0, 1)> 22 6
            Volume b({2, 0, 0}, {1, 3, 2});

            EXPECT_EQ(a.contact(b), 0);
            EXPECT_EQ(b.contact(a), 0);
        }

        {
            // <(0, 0) (0, 2) (0, 1)> 22 6
            Volume a({0, 0, 0}, {1, 3, 2});
            // <(1, 1) (0, 0) (0, 1)> 10 2
            Volume c({1, 0, 0}, {1, 1, 2});

            EXPECT_EQ(a.contact(c), 2);
            EXPECT_EQ(c.contact(a), 2);
        }
        {
            // <(0, 0) (0, 2) (0, 1)> 22 6
            Volume a({0, 0, 0}, {1, 3, 2});
            // <(1, 1) (2, 2) (0, 1)> 10 2
            Volume d({1, 2, 0}, {1, 1, 2});

            EXPECT_EQ(a.contact(d), 2);
            EXPECT_EQ(d.contact(a), 2);
        }
        {
            // <(0, 0) (0, 2) (0, 1)> 22 6
            Volume a({0, 0, 0}, {1, 3, 2});
            // <(1, 1) (1, 1) (0, 0)> 6 1
            Volume e({1, 1, 0}, {1, 1, 1});

            EXPECT_EQ(a.contact(e), 1);
            EXPECT_EQ(e.contact(a), 1);
        }
        {
            // <(2, 2) (0, 2) (0, 1)> 22 6
            Volume b({2, 0, 0}, {1, 3, 2});
            // <(1, 1) (0, 0) (0, 1)> 10 2
            Volume c({1, 0, 0}, {1, 1, 2});

        }
        {
            // <(2, 2) (0, 2) (0, 1)> 22 6
            Volume b({2, 0, 0}, {1, 3, 2});

            // <(1, 1) (2, 2) (0, 1)> 10 2
            Volume d({1, 2, 0}, {1, 1, 2});

        }
        {
            // <(2, 2) (0, 2) (0, 1)> 22 6
            Volume b({2, 0, 0}, {1, 3, 2});

            // <(1, 1) (1, 1) (0, 0)> 6 1
            Volume e({1, 1, 0}, {1, 1, 1});

        }
        {
            // <(1, 1) (0, 0) (0, 1)> 10 2
            Volume c({1, 0, 0}, {1, 1, 2});

            // <(1, 1) (2, 2) (0, 1)> 10 2
            Volume d({1, 2, 0}, {1, 1, 2});

        }
        {
            // <(1, 1) (0, 0) (0, 1)> 10 2
            Volume c({1, 0, 0}, {1, 1, 2});

            // <(1, 1) (1, 1) (0, 0)> 6 1
            Volume e({1, 1, 0}, {1, 1, 1});

        }
        {
            // <(1, 1) (2, 2) (0, 1)> 10 2
            Volume d({1, 2, 0}, {1, 1, 2});

            // <(1, 1) (1, 1) (0, 0)> 6 1
            Volume e({1, 1, 0}, {1, 1, 1});

        }

    }
}

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

TEST(volume_tests, set_surface_operation){
    {
        Volume a({0, 0, 0}, {1, 1, 1});
        Volume b({1, 0, 0}, {1, 1, 1});
        Volume c({2, 0, 0}, {1, 1, 1});

        ASSERT_EQ(surface({a, b, c}), 3 * 4 + 2);
    }

    {
        Volume a({0, 0, 0}, {1, 1, 1});
        Volume b({1, 0, 0}, {1, 1, 1});
        Volume c({0, 1, 0}, {1, 1, 1});

        ASSERT_EQ(surface({a, b, c}), 3 + 3 + 2 * 4);
    }

    {
        Volume a({0, 0, 0}, {1, 1, 1});
        Volume b({1, 0, 0}, {1, 1, 1});
        Volume c({0, 1, 0}, {1, 1, 1});
        Volume d({-1, 0, 0}, {1, 1, 1});

        ASSERT_EQ(surface({a, b, c, d}), 4 * 2 + 3 * 2 + 2 * 2);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 5; xx++)
            for(int yy = 0; yy < 1; yy++)
                for(int zz = 0; zz < 1; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 5 * 4 + 2);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 10; xx++)
            for(int yy = 0; yy < 1; yy++)
                for(int zz = 0; zz < 1; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 10 * 4 + 2);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 2; xx++)
            for(int yy = 0; yy < 2; yy++)
                for(int zz = 0; zz < 1; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 4 * 2 + 2 * 4);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 2; xx++)
            for(int yy = 0; yy < 2; yy++)
                for(int zz = 0; zz < 1; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));
        set.push_back(Volume({2, 0, 0}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 5 * 2 + 3 * 2 + 2 * 2);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 2; xx++)
            for(int yy = 0; yy < 3; yy++)
                for(int zz = 0; zz < 1; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 6 + 6 + 3 + 3 + 2 + 2);
    }


    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 2; xx++)
            for(int yy = 0; yy < 2; yy++)
                for(int zz = 0; zz < 2; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 2 * 2 * 6);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 3; xx++)
            for(int yy = 0; yy < 3; yy++)
                for(int zz = 0; zz < 2; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 3 * 3 * 2 + 2 * 3 * 4);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 3; xx++)
            for(int yy = 0; yy < 3; yy++)
                for(int zz = 0; zz < 3; zz++)
                    set.push_back(Volume({xx, yy, zz}, {1, 1, 1}));

        EXPECT_EQ(surface(set), 3 * 3 * 6);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 3; xx++)
            for(int yy = 0; yy < 3; yy++)
                for(int zz = 0; zz < 3; zz++)
                    set.push_back(Volume({xx*3, yy*3, zz*3}, {3, 3, 3}));

        EXPECT_EQ(surface(set), 9 * 9 * 6);
    }

    {
        std::vector<Volume> set;
        for(int xx = 0; xx < 3; xx++)
            for(int yy = 0; yy < 3; yy++)
                for(int zz = 0; zz < 3; zz++)
                    set.push_back(Volume({xx*3, yy*3, zz*3}, {3, 3, 3}));
        std::swap(set[13], set.back());
        set.pop_back();

        EXPECT_EQ(surface(set), 9 * 9 * 6 + 6 * 9);
    }
}

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
