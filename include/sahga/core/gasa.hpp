#pragma once

#include <memory>
#include <sahga/structures/dataset.hpp>
#include <sahga/structures/graph.hpp>
#include <sahga/utils/common.hpp>
#include <sahga/utils/random.hpp>

//----------------------------------------------------------------------------------------------
// Chromosome structure that defines the format of each gene that compose a chromosome
//----------------------------------------------------------------------------------------------
struct GeneFormat {
  double min;  // Smallest possible value for the gene.
  double max;  // Largest possible value for the gene.
};

//----------------------------------------------------------------------------------------------
// Estrutura de dados que compõe um gene
// Datastructure that defines a gene
//----------------------------------------------------------------------------------------------
struct Gene {
  GeneFormat format;  // Format of the gene.
  double value;       // Value of the gene.
};

//----------------------------------------------------------------------------------------------
// Estrutura de dados que define um cromossomo
//----------------------------------------------------------------------------------------------
struct Chromosome {
  double fitness;           // Chromosome's fitness.
  std::vector<Gene> genes;  // Um cromossomo com N genes de acordo com a estrutura TGene
};

//----------------------------------------------------------------------------------------------
// Genetic Algorithm/Simulated Annealing
//----------------------------------------------------------------------------------------------
class GASA {
private:
  double normalizeFitnessFactor;
  std::unique_ptr<Random> _random;

  void sortChromosomes(std::vector<Chromosome> &chromosomes, int32_t start, int32_t end);
  int32_t partitionChromosomes(std::vector<Chromosome> &chromosomes, int32_t start, int32_t end);

public:
  enum class ModelType { LINEAR, QUADRATIC, LAG };
  enum class ObjectiveType { MINSQT, MINERR, MINBOTH };
  enum class SAHGAParameter { DEFAULT, FAST, HARD, ULTRA, HIGHPOP };

  Graph *graph;
  Dataset *dataset;
  int32_t populationSize, eliteSize, geneSize, maxGenerations, maxIterations;
  ModelType modelType;
  ObjectiveType objectiveFunction;

  // Genetic Algorithm constraints
  float mutationRate, crossoverRate;
  // Simulated Annealing constraints
  float currentTemperature, minimumTemperature, maximumTemperature, coolingRate, epsilon;

  // Genetic Algorithm parameters
  GeneFormat geneFormat;
  std::vector<GeneFormat> chromosomeFormat;
  std::vector<Chromosome> population;
  Chromosome bestChromosome;

  GASA(const Graph &graph, const Dataset &dataset, const ModelType &modelType = ModelType::LINEAR,
       const ObjectiveType &objectiveFunction = ObjectiveType::MINSQT);
  ~GASA();

  GASA *setSAHGAParameters(const SAHGAParameter &parameters = SAHGAParameter::DEFAULT);
  GASA *setPopulationSize(const int32_t &populationSize = 50);
  GASA *setGeneRange(const float &min = -4, const float &max = 4);
  GASA *setGeneRange(GeneFormat &geneFormat, const float &min = -4, const float &max = 4);
  GASA *setGenerations(const int32_t &generations = 3);
  GASA *setMinimumTemperature(const float &minimumTemperature = 0.001);
  GASA *setMaximumTemperature(const float &maximumTemperature = 3);
  GASA *setCoolingRate(const float &coolingRate = 0.9);
  GASA *setIterations(const int32_t &iterations = 3);
  GASA *setEliteSize(const int32_t &eliteSize = 1);
  GASA *setCrossoverRate(const float &crossoverRate = 80);
  GASA *setMutationRate(const float &mutationRate = 1);
  GASA *setEpsilon(const float &episilon = 0.1);

  GASA *run();

  void createPopulation();
  Chromosome createChromosome(const int32_t &nGenes, const std::vector<GeneFormat> &format);
  Gene createGene(const double &min, const double &max);

  void calculateFitnessGA();
  void evolveGA();
  int32_t selection(const double &fitnessSum);
  Chromosome crossoverChromosomeGA(const Chromosome &chromosome1, const Chromosome &chromosome2);
  Gene crossoverGeneAG(const Gene &gene1, const Gene &Gene2, const float &weight);
  void mutateChromosomeGA(Chromosome &chromosome);
  void mutateGeneGA(Gene &gene);

  void calculateFitnessSA();
  void evolveSA();
  void mutateChromosomeSA(Chromosome &chromosome);
  void mutateGeneSA(Gene &gene);
  void resetCurrentTemperature();

  double calculateChromosomeFitness(const Chromosome &chromosome);
};
