
#include <iostream>
#include <vector>

#include "GasSpace.hpp"


int main(){
    GasSpace space;

    space.clear(Volume({0, 0, 0}, {50, 50, 50}));
    space.clear(Volume({50, 0, 0}, {5, 5, 5}));
    space.clear(Volume({55, 0, 0}, {50, 50, 50}));

    std::cout << space.describe();

    auto print = [&](){
        std::cout  << space.air_at({20, 20, 20}) << " " << space.air_at({30, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;
    };

    for(uint ii = 0; ii < 10; ii++){
        space.step(0.1);
    }

    print();
    space.add_air({1, 1, 1}, 1000000);

    print();

    for(uint ii = 0; ii < 10; ii++){
        space.step(0.1);
    }
    print();

    std::cout << "-------------------" << std::endl;
    std::cout << "blocking" << std::endl;
    space.block(Volume({25, 0, 0}, {1, 50, 50}));
    std::cout << space.describe();
    for(uint ii = 0; ii < 10; ii++){
        print();
        space.step(0.1);
    }
    print();


    std::cout << "-------------------" << std::endl;
    std::cout << "blocking" << std::endl;
    space.block(Volume({0, 0, 0}, {25, 50, 50}));
    std::cout << space.describe();
    for(uint ii = 0; ii < 10; ii++){
        print();
        space.step(0.1);
    }
    print();

    std::cout << "-------------------" << std::endl;
    std::cout << "blocking" << std::endl;
    space.block(Volume({55, 5, 25}, {50, 50, 1}));
    std::cout << space.describe();


    return 0;
}
