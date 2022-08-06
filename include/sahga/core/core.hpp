#pragma once

#include <sahga/core/gasa.hpp>
#include <sahga/structures/dataset.hpp>
#include <sahga/structures/graph.hpp>
#include <sahga/structures/layer.hpp>
#include <sahga/utils/common.hpp>
#include <sahga/utils/random.hpp>

class SAHGACore {
private:
  std::unique_ptr<Random> _random;

  int16_t id;
  int32_t population, generations;
  float radius;
  int8_t objectiveFunction;

public:
  SAHGACore(int16_t id, int32_t population, int32_t generations, float radius,
            int8_t objectiveFunction);
  ~SAHGACore();

  SAHGACore* separateTrainTest(const std::string& filename, double ratio = 75.0,
                               bool stratify = false);
  SAHGACore* generateMPG();
  SAHGACore* extractLayers(const std::string& filename);
  SAHGACore* mergeData(const std::string& mpgFilename, const std::string& layersFilename);
  SAHGACore* adjustModel(int32_t modelType, int32_t objectType, const std::string& filename,
                         const bool& normalize = true);
};
