#include <sahga/utils/random.hpp>

Random::Random(double min, double max)
    : _randomDevice(), _generator(_randomDevice()), _distribution(min, max) {}

double Random::next() { return _distribution(_generator); }
double Random::operator()() { return next(); }
