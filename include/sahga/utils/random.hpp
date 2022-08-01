#pragma once

#include <random>
#include <sahga/utils/random.hpp>

class Random {
private:
  std::random_device _randomDevice;
  std::mt19937 _generator;
  std::uniform_real_distribution<double> _distribution;

public:
  Random(double min, double max);

  double next();
  double operator()();
};
