
#include <iostream>
#include <vector>

#include "GasSpace.hpp"

int main(){

    GasSpace space;

    space.clear(Volume({0, 0, 0}, {50, 50, 50}));
    space.clear(Volume({55, 0, 0}, {50, 50, 50}));

    std::cout << space.size() << std::endl;
    std::cout << space.describe();

    for(uint ii = 0; ii < 10; ii++){
        space.step(0.1);
    }

    std::cout << space.air_at({20, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;

    space.add_air({1, 1, 1}, 1000000);

    std::cout << space.air_at({20, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;

    for(uint ii = 0; ii < 10; ii++){
        space.step(0.1);
    }
    std::cout << space.air_at({20, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;

    space.clear(Volume({50, 0, 0}, {5, 5, 5}));
    std::cout << space.size() << std::endl;
    std::cout << space.describe();

    for(uint ii = 0; ii < 10; ii++){
        std::cout << space.air_at({20, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;
        space.step(0.1);
    }
    std::cout << space.air_at({20, 20, 20}) << " " << space.air_at({75, 20, 20}) << std::endl;



    return 0;
}
