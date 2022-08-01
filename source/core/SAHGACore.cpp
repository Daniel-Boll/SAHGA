#include <sahga/core/core.hpp>

static std::string getServerPathTo(const std::string& file) {
  const std::string cd = Utils::filemanagement::getRootDirectory("sahga-api-xmake");
  return fmt::format("{}/assets/server-info/{}", cd, file);
}

static std::string getUserPathTo(int32_t userId, const std::string& file) {
  const std::string cd = Utils::filemanagement::getRootDirectory("sahga-api-xmake");
  return fmt::format("{}/assets/user-info/{:04}/{}", cd, userId, file);
}

SAHGACore::SAHGACore() : _random(std::make_unique<Random>(.0, 1.)) {
  const std::string speciesPoints = getServerPathTo("PtsFurcata.txt");
  const std::string geographicalLayers = getServerPathTo("layers");

  const int32_t userId = 1;

  const std::string generalProximationMatrix = getUserPathTo(userId, "gpm.txt");
  const std::string geographicalVariables = getUserPathTo(userId, "layersData.txt");

  const std::string mergedMatrix = getUserPathTo(userId, "mergedData.txt");

  const auto modelType = GASA::ModelType::QUADRATIC;
  const auto objectiveType = GASA::ObjectiveType::MINBOTH;

  try {
    separateTrainTest(speciesPoints)
        ->generateMPG()
        ->extractLayers(geographicalLayers)
        ->mergeData(generalProximationMatrix, geographicalVariables)
        ->adjustModel((int32_t)modelType, (int32_t)objectiveType, mergedMatrix);
  } catch (const std::exception& e) {
    fmt::print("Error\n");
    fmt::print("{}\n", e.what());
  }
}

/*
 * Separate original dataset in Train and Test instances.
 *
 * @param { std::string } **filepath** - Path to the dataset.
 * @param { double } **ratio** - Ratio of the distribution. **Default** as 75%
 * so 75% of the input file goes to Training and the remaining 25% goes to Test.
 * @param { bool } **stratify** - Preserves the same proportions of
 * examples in each class as observed in the original dataset. **Default** as
 * false.
 *
 * @return
 * */
SAHGACore* SAHGACore::separateTrainTest(const std::string& filepath, double ratio, bool stratify) {
  const std::string cd = Utils::filemanagement::getRootDirectory("sahga-api-xmake");
  std::string trainFilename = fmt::format("{}/assets/user-info/0001/train.txt", cd);
  std::string testFilename = fmt::format("{}/assets/user-info/0001/test.txt", cd);

  std::vector<std::string> dataPoints;

  // Create all the path needed to store the train and test files
  std::filesystem::create_directories(fmt::format("{}/assets/user-info/0001", cd));

  std::filesystem::exists(trainFilename) ? std::filesystem::remove(trainFilename) : 0;
  std::filesystem::exists(testFilename) ? std::filesystem::remove(testFilename) : 0;

  // Read the file that contains the presence/absence of the species and store
  // the data in the vector *dataPoints*
  ReadFile::Read(dataPoints, filepath);

  // Extract the header and then remove it from the vector
  std::string header = dataPoints.front();
  dataPoints.erase(dataPoints.begin());

  auto engine = std::default_random_engine{};
  std::vector<std::string> shuffledData(dataPoints);
  std::shuffle(shuffledData.begin(), shuffledData.end(), engine);

  int32_t trainSize = static_cast<int32_t>(shuffledData.size() * (ratio / 100));

  // Write ratio percent of the data to the train file and the remaining to the
  // test file.
  std::ofstream os;
  os.open(trainFilename);
  os << header << std::endl;
  for (uint32_t i = 0; i < trainSize; ++i) {
    std::string line = shuffledData[i];
    line.erase(0, line.find('\t'));

    os << (i + 1) << line << "\n";
  }
  os.close();

  os.open(testFilename);
  os << header << std::endl;
  for (uint32_t i = trainSize; i < shuffledData.size(); ++i) {
    std::string line = shuffledData[i];
    line.erase(0, line.find('\t'));

    os << (i + 1 - trainSize) << line << "\n";
  }
  os.close();

  return this;
}

SAHGACore* SAHGACore::generateMPG() {
  const std::string filename
      = fmt::format("{}/assets/user-info/0001/train.txt",
                    Utils::filemanagement::getRootDirectory("sahga-api-xmake"));
  const std::string output
      = fmt::format("{}/assets/user-info/0001/gpm.txt",
                    Utils::filemanagement::getRootDirectory("sahga-api-xmake"));

  auto dataset = std::make_unique<Dataset>();
  ReadFile::Read(*dataset, filename, '\t');

  // TODO: This need to be a input to the program
  const Graph::MPGTypes mpgType = Graph::MPGTypes::HALFRADIUM;
  const double radius = 5;

  std::make_unique<Graph>()
      ->createMPG(*dataset, radius, (int32_t)mpgType)
      ->save(filename, radius, output);

  return this;
}

SAHGACore* SAHGACore::extractLayers(const std::string& folderName) {
  const std::string presenceAusenceFilename
      = fmt::format("{}/assets/user-info/0001/train.txt",
                    Utils::filemanagement::getRootDirectory("sahga-api-xmake"));
  const std::string output
      = fmt::format("{}/assets/user-info/0001/layersData.txt",
                    Utils::filemanagement::getRootDirectory("sahga-api-xmake"));

  auto dataset = std::make_unique<Dataset>();
  ReadFile::Read(*dataset, presenceAusenceFilename, '\t');

  // for every file in the folder (folderName)
  std::vector<Layer*> layers;

  std::for_each(std::filesystem::directory_iterator(folderName),
                std::filesystem::directory_iterator(),
                [&layers](const std::filesystem::directory_entry& entry) {
                  layers.emplace_back((new Layer())->load(entry.path().string()));
                });

  auto layersData = std::make_unique<Dataset>();
  layersData->reset(dataset->rowN, layers.size());

  for (int32_t i = 0; i < dataset->rowN; ++i)
    for (int32_t j = 0; j < layers.size(); ++j)
      layersData->M[i][j] = layers[j]->getValue(dataset->M[i][1], dataset->M[i][2]);

  std::ofstream outputStream(output.c_str());
  outputStream << setiosflags(std::ios::fixed);
  outputStream << "//Dados extraidos do conjunto de layers: " << presenceAusenceFilename << '\n';
  outputStream << "//Formato dos dados --> #id;long;lat;x0;x1;x2;...;xn" << '\n';

  for (int32_t i = 0; i < dataset->rowN; ++i) {
    outputStream << std::setprecision(0) << dataset->M[i][0] << ';';  //#id
    outputStream << std::setprecision(6) << dataset->M[i][1] << ';';  // long
    outputStream << dataset->M[i][2] << ';';                          // lat
    outputStream << std::setprecision(0) << dataset->M[i][3]
                 << ';';  // 0 para ausência e 1 para presença
    outputStream << std::setprecision(3);
    for (int32_t j = 0; j < layers.size(); ++j) outputStream << layersData->M[i][j] << ';';
    outputStream << '\n';
  }

  outputStream.close();
  return this;
}

SAHGACore* SAHGACore::mergeData(const std::string& mpgFilename, const std::string& layersFilename) {
  std::vector<std::string> mpgData, layersData;

  ReadFile::Read(mpgData, mpgFilename);
  ReadFile::Read(layersData, layersFilename);

  if (mpgData.size() != layersData.size())
    throw std::runtime_error("The files have different sizes");

  for (int32_t i = 0; i < mpgData.size(); ++i) {
    int32_t character = 0;
    int32_t variableCounter = 0;

    while (variableCounter < 3) {
      if (layersData[i][character] == ';') ++variableCounter;

      ++character;
    }

    layersData[i] = layersData[i].substr(character, layersData[i].length() - character - 1);
  }

  int32_t Nvariables = 0;

  for (int32_t i = 0; i < layersData[0].length(); ++i)
    if (layersData[0][i] == ';') ++Nvariables;

  std::string mergedDataFilename
      = fmt::format("{}/assets/user-info/0001/mergedData.txt",
                    Utils::filemanagement::getRootDirectory("sahga-api-xmake"));

  std::ofstream mergedDataStream(mergedDataFilename.c_str());

  mergedDataStream << std::setiosflags(std::ios::fixed);
  mergedDataStream << std::setprecision(0) << mpgData.size() << ';' << Nvariables << '\n';

  for (int32_t i = 0; i < mpgData.size(); ++i)
    mergedDataStream << mpgData[i] << layersData[i] << '\n';

  mergedDataStream.close();

  mpgData.clear();
  layersData.clear();

  return this;
}

SAHGACore* SAHGACore::adjustModel(int32_t modelType, int32_t objectiveType,
                                  const std::string& filename, const bool& normalize) {
  auto graph = std::make_unique<Graph>();
  auto dataset = std::make_unique<Dataset>();

  ReadFile::Read(*graph, *dataset, filename, ';');

  // Atualizando as estatísticas da matriz de dados
  dataset->updateStats();

  // Armazena a média e o desvio padrão de cada coluna
  std::vector<double> avg, stdDev;
  for (int32_t i = 0; i < dataset->colN; ++i) {
    avg.emplace_back(dataset->stats[i].avg);
    stdDev.emplace_back(dataset->stats[i].stdDev);
  }

  // Normalizando a matriz de dados
  dataset->normalize(int32_t(normalize));

  auto gasa = (std::make_unique<GASA>(*graph, *dataset, (GASA::ModelType)modelType,
                                      (GASA::ObjectiveType)objectiveType))
                  ->setSAHGAParameters(GASA::SAHGAParameter::HIGHPOP)
                  ->run();

  std::string output = fmt::format("{}/assets/user-info/0001/result.txt",
                                   Utils::filemanagement::getRootDirectory("sahga-api-xmake"));

  std::ofstream outputStream(output.c_str());

  outputStream << std::setiosflags(std::ios::fixed) << std::setprecision(8);
  outputStream << "//Modelo gerado para o arquivo de entrada: " << filename << '\n';
  outputStream << "//Tipo do modelo ajustado:" << '\n';

  if (modelType == (int32_t)GASA::ModelType::LINEAR) outputStream << "LINEAR" << '\n';
  if (modelType == (int32_t)GASA::ModelType::QUADRATIC) outputStream << "QUADRATIC" << '\n';
  if (modelType == (int32_t)GASA::ModelType::LAG) outputStream << "LAG" << '\n';

  if (objectiveType == (int32_t)GASA::ObjectiveType::MINSQT)
    outputStream << "//Aptidao final (Min SQT) = " << gasa->bestChromosome.fitness << '\n';
  if (objectiveType == (int32_t)GASA::ObjectiveType::MINERR)
    outputStream << "//Aptidao final (Min ERR) = " << gasa->bestChromosome.fitness << '\n';
  if (objectiveType == (int32_t)GASA::ObjectiveType::MINBOTH)
    outputStream << "//Aptidao final (Min SQT&ERR) = " << gasa->bestChromosome.fitness << '\n';

  outputStream << "//Saida --> c1;c2;...;cn;constante;[lambda]" << '\n';
  for (int32_t i = 0; i < gasa->geneSize; ++i)
    outputStream << gasa->bestChromosome.genes[i].value << ';';
  outputStream << '\n' << "//Media das variáveis --> Media X0;Media X1;...MediaXn" << '\n';
  for (int32_t i = 0; i < dataset->colN; ++i) outputStream << avg[i] << ';';
  outputStream << '\n' << "//Desvio padrao das variáveis --> s0;s1;...;sn" << '\n';
  for (int32_t i = 0; i < dataset->colN; ++i) outputStream << stdDev[i] << ';';

  outputStream.close();

  return this;
}

SAHGACore::~SAHGACore() { fmt::print("SAHGACore::~SAHGACore()\n"); }
