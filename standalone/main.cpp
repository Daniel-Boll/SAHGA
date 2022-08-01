#include <sahga/core/core.hpp>

int main(int argc, char *argv[]) {
  std::string functionparameters[argc - 1];
  std::copy(argv + 1, argv + argc, functionparameters);

  std::make_unique<SAHGACore>();

  return 0;
}
