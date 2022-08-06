#include <iostream>
#include <sahga/core/core.hpp>

int main(int argc, char* argv[]) {
  std::string functionparameters[5];
  std::copy(argv + 1, argv + argc, functionparameters);

  auto [id, population, generations, radius, objectiveFunction] = functionparameters;

  std::make_unique<SAHGACore>(std::stoi(id), std::stoi(population), std::stoi(generations),
                              std::stof(radius), std::stoi(objectiveFunction));

  return 0;
}
