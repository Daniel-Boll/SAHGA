#include <omp.h>

#include <cmath>
#include <iostream>
#include <sahga/core/gasa.hpp>
#include <sahga/utils/random.hpp>
#include <sahga/utils/utils.hpp>

//----------------------------------------------------------------------------------------------
// Fitness calculation for the chromossome
//----------------------------------------------------------------------------------------------
double GASA::calculateChromosomeFitness(const Chromosome &chromosome) {
  double fitness = 0;

  switch (modelType) {
    case ModelType::LINEAR: {
      int32_t independentVariablesNumber = geneSize - 1;

      // Para todos os dados de entrada
      for (int32_t i = 0; i < dataset->rowN; ++i) {
        double chromosomeEstimation = 0;

        // Para todas as variáveis independentes
        for (int32_t j = 0; j < independentVariablesNumber; ++j) {
          double sumN = 0;
          double sumD = 0;

          // Para todos os k vizinhos do objeto i
          for (int32_t k = 0; k < graph->node[i].nRel; ++k) {
            // Totaliza a influencia dos k vizinhos de i pelos pesos dos
            // relacionamentos
            sumN += (graph->node[i].edge[k].weight
                     * dataset->M[graph->node[i].edge[k].nodeId - 1][j + 1]);
            // Soma dos pesos dos k relacionamentos entre i e seus vizinhos
            sumD += graph->node[i].edge[k].weight;
          }

          // Acumula o termo correspondente à variável j (Pj * Média(Xj))
          chromosomeEstimation += (chromosome.genes[j].value * (sumN / sumD));
        }

        // Acumula a constante do modelo
        chromosomeEstimation += chromosome.genes[geneSize - 1].value;

        // Incrementa em fitness a parcela de erro ocorrido na amostra i
        switch (objectiveFunction) {
          case ObjectiveType::MINSQT:  // MINSQT = Minimizar a soma do quadrado dos desvios (todos
                                       // os modelos)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            break;
          case ObjectiveType::MINERR:  // MINERR = Minimizar a quantidade de erros de
                                       // Omissão/Comissão (Dist. de espécies)
                                       // Avaliado como ausência mas era presença (False Negative =
                                       // Omission Error) ou Avaliado como presença mas era ausência
                                       // ((False Positive = Comission Error)
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              ++fitness;
            break;
          case ObjectiveType::MINBOTH:  // MINBOTH = Minimizar tanto o SQT quanto os erros de
                                        // Omissão/Comissão (Dist. de espécies)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              fitness += epsilon;
            break;
        }
      }
      break;  // case ModelType::LINEAR
    }

    case ModelType::QUADRATIC: {
      // Metade dos coeficientes de variáveis (-1 desconta o termo constante)
      int32_t independentVariablesNumber = (geneSize - 1) / 2;

#pragma omp parallel for firstprivate(independentVariablesNumber) reduction(+ : fitness)
      // #pragma omp parallel for reduction(+ : fitness) num_threads(8)
      for (int32_t i = 0; i < dataset->rowN; ++i) {
        // fmt::print("({}) - id: {}\n", i, omp_get_thread_num());
        double chromosomeEstimation = 0;

        // Para todas as variáveis independentes
        for (int32_t j = 0; j < independentVariablesNumber; ++j) {
          double sumN = 0;
          double sumD = 0;

          // fmt::print("({}) Number of relationship: {}\n", graph->node[i].nodeId,
          // graph->node[i].nRel);

          // Para todos os k vizinhos do objeto i
          for (int32_t k = 0; k < graph->node[i].nRel; ++k) {
            // Totaliza a influencia dos k vizinhos de i pelos pesos dos
            // relacionamentos
            sumN += (graph->node[i].edge[k].weight
                     * dataset->M[graph->node[i].edge[k].nodeId - 1][j + 1]);
            // Soma dos pesos dos k relacionamentos entre i e seus vizinhos
            sumD += graph->node[i].edge[k].weight;
          }

          // Acumula o termo correspondente à variável j (Pj * Média(Xj))
          chromosomeEstimation += (chromosome.genes[2 * j].value * pow((sumN / sumD), 2)
                                   + chromosome.genes[2 * j + 1].value * (sumN / sumD));
        }

        // Acumula a constante do modelo
        chromosomeEstimation += chromosome.genes[geneSize - 1].value;

        // fmt::print("({}) Chromosome estimation: {}\n", graph->node[i].nodeId,
        // chromosomeEstimation);

        // Incrementa em fitness a parcela de erro ocorrido na amostra i
        switch (objectiveFunction) {
          case ObjectiveType::MINSQT:  // MINSQT = Minimizar a soma do quadrado dos desvios (todos
                                       // os modelos)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            break;
          case ObjectiveType::MINERR:  // MINERR = Minimizar a quantidade de erros de
                                       // Omissão/Comissão (Dist. de espécies)
                                       // Avaliado como ausência mas era presença (False Negative =
                                       // Omission Error) ou Avaliado como presença mas era ausência
                                       // ((False Positive = Comission Error)
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              ++fitness;
            break;
          case ObjectiveType::MINBOTH:  // MINBOTH = Minimizar tanto o SQT quanto os erros de
                                        // Omissão/Comissão (Dist. de espécies)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              fitness += epsilon;
            break;
        }
      }
      break;  // case ModelType::QUADRATIC
    }
    case ModelType::LAG: {
      int32_t independentVariablesNumber
          = geneSize - 2;  // independentVariablesNumber = número de variáveis independentes;
                           // desconta a constante e o lambda

      // Para todos os dados de entrada
      for (int32_t i = 0; i < dataset->rowN; ++i) {
        double chromosomeEstimation = 0;
        // Acumulando as contribuições em Y = (Coeficiente * Variável
        // Independente)
        for (int32_t j = 0; j < independentVariablesNumber; ++j)
          chromosomeEstimation += (chromosome.genes[j].value * dataset->M[i][j + 1]);
        // Acumulando a constante do modelo
        chromosomeEstimation += chromosome.genes[geneSize - 2].value;
        // Acumulando a contribuição do termo espacializado
        double sumN = 0;
        double sumD = 0;

        // Para todos os j vizinhos do objeto i
        for (int32_t j = 0; j < graph->node[i].nRel; ++j) {
          if (graph->node[i].nodeId != graph->node[i].edge[j].nodeId) {
            // Totaliza a influência dos j vizinhos de i pelos pesos dos
            // relacionamentos
            sumN += (graph->node[i].edge[j].weight
                     * dataset->M[graph->node[i].edge[j].nodeId - 1][0]);
            // Soma dos pesos dos relacionamentos entre i e seus j vizinhos
            sumD += graph->node[i].edge[j].weight;
          }
        }

        // Média ponderada das variáveis dependentes relacionadas a Y[i]
        double average = (sumD == 0) ? 0 : sumN / sumD;

        // Acumula a contribuição do termo espacial --> lambda * average dos Y[i]
        chromosomeEstimation += (chromosome.genes[geneSize - 1].value * average);

        // Incrementa em fitness a parcela de erro ocorrido na amostra i
        switch (objectiveFunction) {
          case ObjectiveType::MINSQT:  // MINSQT = Minimizar a soma do quadrado dos desvios (todos
                                       // os modelos)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            break;
          case ObjectiveType::MINERR:  // MINERR = Minimizar a quantidade de erros de
                                       // Omissão/Comissão (Dist. de espécies)
                                       // Avaliado como ausência mas era presença (False Negative =
                                       // Omission Error) ou Avaliado como presença mas era ausência
                                       // ((False Positive = Comission Error)
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              ++fitness;
            break;
          case ObjectiveType::MINBOTH:  // MINBOTH = Minimizar tanto o SQT quanto os erros de
                                        // Omissão/Comissão (Dist. de espécies)
            fitness += pow((dataset->M[i][0] - chromosomeEstimation), 2);
            if (((chromosomeEstimation < 0.5) && (dataset->M[i][0] == 1))
                || ((chromosomeEstimation >= 0.5) && (dataset->M[i][0] == 0)))
              fitness += epsilon;
            break;
        }
      }
      break;  // case LAG
    }
  }  // end switch

  return (fitness);
}

//----------------------------------------------------------------------------------------------
// Simulated Annealing
// Reajusta o valor de TAtual. Necessário em cada ciclo do AG/SA.
//----------------------------------------------------------------------------------------------
void GASA::resetCurrentTemperature() { currentTemperature = maximumTemperature; }

//----------------------------------------------------------------------------------------------
// Simulated Annealing
// Rotina para mutar aleatoriamente o valor de um gene. Uma pequena perturbação
// no valor.
//----------------------------------------------------------------------------------------------
void GASA::mutateGeneSA(Gene &gene) {
  double value;

  value = gene.value + (_random->next() - 0.5);
  // gene.value = (value < Gene.ValMin) ? Gene.ValMin : (value > Gene.ValMax) ? Gene.ValMax : value;
  gene.value = (value < gene.format.min)   ? gene.format.min
               : (value > gene.format.max) ? gene.format.max
                                           : value;
}

//----------------------------------------------------------------------------------------------
// Simulated Annealing
// Efetua a mutacao do cromossomo realizando a mutacao dos genes
//----------------------------------------------------------------------------------------------
void GASA::mutateChromosomeSA(Chromosome &chromosome) {
  for (int32_t i = 0; i < geneSize; ++i) mutateGeneSA(chromosome.genes[i]);
}

//----------------------------------------------------------------------------------------------
// Simulated Annealing
// Realiza a evolução da população atual, aplicando o Simulated Annealing.
//----------------------------------------------------------------------------------------------
void GASA::evolveSA() {
  std::vector<Chromosome> newPopulation;

  // Inicializa vetor da nova população
  newPopulation.resize(populationSize);

  // Realiza a mutação da nova população
  for (int32_t i = 0; i < maxIterations; ++i) {
    for (int32_t j = 0; j < populationSize; ++j) {
      newPopulation[j] = population[j];
      mutateChromosomeSA(newPopulation[j]);
      newPopulation[j].fitness = calculateChromosomeFitness(newPopulation[j]);

      double delta = newPopulation[j].fitness - population[j].fitness;

      // Para maximizar --> Delta >= 0; Para minimizar --> Delta <= 0
      if (delta <= 0) population[j] = newPopulation[j];
      // Para maximizar --> exp(Delta/TAtual); Para minimizar -->
      // exp(-Delta/TAtual)
      else if (_random->next() < exp(-delta / currentTemperature))
        population[j] = newPopulation[j];
    }
  }
  // fmt::print("[B8]\n");
}

//----------------------------------------------------------------------------------------------
// Simulated Annealing
// Faz a avaliação da população atual, identificando o melhor indivíduo
//----------------------------------------------------------------------------------------------
void GASA::calculateFitnessSA() {
  // Resfriamento do Simulated Annealing
  currentTemperature = currentTemperature * coolingRate;
  // Para maximizar --> ordenação decrescente; Para minimizar --> ordenação
  // crescente
  sortChromosomes(population, 0, populationSize - 1);

  // Para maximizar --> >; Para minimizar --> <
  if (population[0].fitness < bestChromosome.fitness) {
    bestChromosome = population[0];
  }
}

void GASA::sortChromosomes(std::vector<Chromosome> &chromosomes, int32_t start, int32_t end) {
  if (start < end) {
    int32_t j = partitionChromosomes(chromosomes, start, end);
    sortChromosomes(chromosomes, start, j - 1);
    sortChromosomes(chromosomes, j + 1, end);
  }
}

int32_t GASA::partitionChromosomes(std::vector<Chromosome> &chromosomes, int32_t start,
                                   int32_t end) {
  Chromosome Temp;
  Chromosome Pivo = chromosomes[start];

  int32_t i = start;
  int32_t j = end + 1;

  while (true) {
    do {
      ++i;
    } while ((i <= end)
             && (chromosomes[i].fitness <= Pivo.fitness));  // crescente --> <=; decrescente --> >=

    do {
      --j;
    } while (chromosomes[j].fitness > Pivo.fitness);  // crescente --> >; decrescente --> <

    if (i >= j) break;
    Temp = chromosomes[i];
    chromosomes[i] = chromosomes[j];
    chromosomes[j] = Temp;
  }

  Temp = chromosomes[start];
  chromosomes[start] = chromosomes[end];
  chromosomes[end] = Temp;
  return j;
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Rotina para mutar aleatoriamente um Gene - Algoritmo Genético
//----------------------------------------------------------------------------------------------
void GASA::mutateGeneGA(Gene &gene) {
  if (_random->next() < mutationRate) {
    gene.value = _random->next() * (gene.format.max - gene.format.min) + gene.format.min;
  }
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Efetua a mutação do cromossomo realizando a mutação de cada um de seus genes
//----------------------------------------------------------------------------------------------
void GASA::mutateChromosomeGA(Chromosome &chromosome) {
  for (int32_t i = 0; i < geneSize; ++i) mutateGeneGA(chromosome.genes[i]);
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Rotina para efetuar o crossover aritmético entre genes. Recebe como
// parâmetros a taxa de crossover, os genes que serão cruzados e o peso de
// ponderação dos genes
//----------------------------------------------------------------------------------------------
Gene GASA::crossoverGeneAG(const Gene &gene1, const Gene &gene2, const float &weight) {
  Gene gene = gene1;

  if (_random->next() < crossoverRate)
    gene.value = weight * gene1.value + (1 - weight) * gene2.value;

  return (gene);
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Retorna um cromossomo que é o resultado do crossover entre os cromossomos
// Para gerar o novo cromossomo efetua o crossover entre seus genes
//----------------------------------------------------------------------------------------------
Chromosome GASA::crossoverChromosomeGA(const Chromosome &chromosome1,
                                       const Chromosome &chromosome2) {
  Chromosome chromosome;

  chromosome.genes.resize(geneSize);

  for (int32_t i = 0; i < geneSize; ++i)
    chromosome.genes[i]
        = crossoverGeneAG(chromosome1.genes[i], chromosome2.genes[i], _random->next());

  return (chromosome);
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Realiza a seleção de indivíduos da população para aplicar os operadores
// genéticos
//----------------------------------------------------------------------------------------------
int32_t GASA::selection(const double &fitnessSum) {
  float step = 0;
  int32_t i = -1;
  double random = _random->next() * fitnessSum;
  // double random = fitnessSum;
  // fmt::print("fs: {}\n", fitnessSum);
  // fmt::print("r: {}\n", random);

  do {
    ++i;
    // Para maximizar --> Pop[i] - FatorTrans; Para minimizar --> FatorTrans -
    // Pop[i]
    step += (normalizeFitnessFactor - population[i].fitness);
    // fmt::print("{} {} {}\n", random, step, step < random);
  } while ((step < random) && (i < (int32_t)(populationSize - 1)));

  return i;
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Realiza a evolução da população atual. Aplicando os operadores genéticos.
//----------------------------------------------------------------------------------------------
void GASA::evolveGA() {
  std::vector<Chromosome> newPopulation;

  // Inicializa vetor da nova população
  newPopulation.resize(populationSize);

  // fmt::print("[B1]\n");
  // Copia os elementos da elite
  for (int32_t i = 0; i < eliteSize; ++i) newPopulation[i] = population[i];
  int32_t newPopulationSize = eliteSize;

  // Calcula o somatório do Fitness - Processo de seleção
  double fitnessSum = 0;

  // fmt::print("[B2]\n");
  for (int32_t i = 0; i < populationSize; ++i) {
    // Para maximizar --> Pop[i] - FatorTrans; Para minimizar --> FatorTrans -
    // Pop[i] O uso do fator de translação elimina o problema decorrente de
    // fitness negativo
    fitnessSum += (normalizeFitnessFactor - population[i].fitness);
    // fmt::print("[{}] {} {}\n", i, normalizeFitnessFactor, fitnessSum);
  }

  // Cruza elementos da população atual até completar a nova população
  // fmt::print("[B3]\n");
  while (newPopulationSize < populationSize) {
    int32_t selection1, selection2;

    // Seleciona dois indivíduos distintos da população para realizar o
    // cruzamento
    do {
      // Aqui pode ocorrer problema -- CSel1 sempre igual a CSel2
      // Há muitos indivíduos com fitness nulo ou muito pequenos. Sobressai um
      // único indivíduo com bom fitness. A seleção então sempre escolhe esse
      // único bom indivíduo. Solução --> A população inicial não deve ser muito
      // pequena --> >=20 indivíduos
      selection1 = selection(fitnessSum);
      selection2 = selection(fitnessSum);

      // fmt::print("{} {}\n", selection1, selection2);
    } while (selection1 == selection2);

    // Cruza o indivíduo CSel1 com CSel2
    newPopulation[newPopulationSize]
        = crossoverChromosomeGA(population[selection1], population[selection2]);
    ++newPopulationSize;

    // Se ainda couber mais um indivíduo na nova população
    if (newPopulationSize < populationSize) {
      // Cruza o indivíduo CSel2 com CSel1
      newPopulation[newPopulationSize]
          = crossoverChromosomeGA(population[selection2], population[selection1]);
      ++newPopulationSize;
    }
  }

  // Realiza a mutação da nova população
  for (int32_t i = eliteSize; i < populationSize; ++i) mutateChromosomeGA(newPopulation[i]);

  // Copiando a nova população
  for (int32_t i = 0; i < populationSize; ++i) population[i] = newPopulation[i];
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Faz a avaliação da população atual, identificando o melhor indivíduo
//----------------------------------------------------------------------------------------------
void GASA::calculateFitnessGA() {
  for (int32_t i = 0; i < populationSize; ++i)
    population[i].fitness = calculateChromosomeFitness(population[i]);

  // Para maximizar --> ordenação decrescente; Para minimizar --> ordenação
  // crescente
  sortChromosomes(population, 0, populationSize - 1);

  // Para maximizar --> (-1); Para minimizar --> (+1)
  normalizeFitnessFactor = population[populationSize - 1].fitness + 1;

  // Para maximizar --> >; Para minimizar --> <
  if (population[0].fitness < bestChromosome.fitness) {
    bestChromosome = population[0];
  }
}

//----------------------------------------------------------------------------------------------
// Construção de um Gene - Um Gene é um valor numérico entre Min e Max
// representando uma variável
//----------------------------------------------------------------------------------------------
Gene GASA::createGene(const double &min, const double &max) {
  Gene gene;

  gene.format.min = min;
  gene.format.max = max;
  gene.value = _random->next() * (max - min) + min;

  // fmt::print("Gene {}\n", gene.value);

  return (gene);
}

//----------------------------------------------------------------------------------------------
// Construção de um chromosomessomo. Com os parâmetros dos genes setados em Ctrl cria
// o cromossomo.
//----------------------------------------------------------------------------------------------
Chromosome GASA::createChromosome(const int &nGenes, const std::vector<GeneFormat> &format) {
  Chromosome chromosome;

  chromosome.genes.resize(nGenes);
  for (int32_t i = 0; i < nGenes; ++i)
    chromosome.genes[i] = createGene(format[i].min, format[i].max);

  return (chromosome);
}

//----------------------------------------------------------------------------------------------
// Cria a população inicial de indivíduos
//----------------------------------------------------------------------------------------------
void GASA::createPopulation() {
  std::vector<GeneFormat>().swap(chromosomeFormat);

  for (int32_t i = 0; i < geneSize; ++i) chromosomeFormat.push_back(geneFormat);

  population.resize(populationSize);
  for (int32_t i = 0; i < populationSize; ++i)
    population[i] = createChromosome(geneSize, chromosomeFormat);
}

//----------------------------------------------------------------------------------------------
// Algoritmo Genético
// Executa o Algoritmo Hibrido repetindo o processo NumCiclos vezes
//----------------------------------------------------------------------------------------------
GASA *GASA::run() {
  createPopulation();    // Criando a população inicial
  calculateFitnessGA();  // Avaliando a população inicial

  for (int32_t i = 0; i < maxGenerations; i++) {
    // fmt::print("Generation {}\n", i);
    std::cout << "Generation " << i << std::endl;
    evolveGA();
    // fmt::print("Evolved\n");
    calculateFitnessGA();
    // fmt::print("Calculated fitness GA\n");

    // fmt::print("Beginning SA\n");
    while (currentTemperature > minimumTemperature) {
      evolveSA();
      calculateFitnessSA();
    }
    // fmt::print("End SA\n");

    population[0] = bestChromosome;  // Restaura o melhor indivíduo pois o SA pode tê-lo modificado
    resetCurrentTemperature();       // Reinicializa --> TAtual = TMax
  }

  return this;
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro epsilon. value adicionado ao Fitness quando ponto
// avaliado como AP ou PA.
//----------------------------------------------------------------------------------------------
GASA *GASA::setEpsilon(const float &epsilon) {
  this->epsilon = epsilon;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro taxa de mutação utilizado no AG.
//----------------------------------------------------------------------------------------------
GASA *GASA::setMutationRate(const float &mutationRate) {
  this->mutationRate = mutationRate;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro taxa de cruzamento utilizado no AG.
//----------------------------------------------------------------------------------------------
GASA *GASA::setCrossoverRate(const float &crossoverRate) {
  this->crossoverRate = crossoverRate;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro tamanho da elite --> Número de elementos preservados em
// cada ciclo AG/SA.
//----------------------------------------------------------------------------------------------
GASA *GASA::setEliteSize(const int &eliteSize) {
  this->eliteSize = eliteSize;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta quantas vezes o Simulated Annealing será executado em cada ciclo
// AG/SA.
//----------------------------------------------------------------------------------------------
GASA *GASA::setIterations(const int &iterations) {
  this->maxIterations = iterations;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro Alfa do Simulated Annealing --> Fator de redução da
// temperatura.
//----------------------------------------------------------------------------------------------
GASA *GASA::setCoolingRate(const float &setCoolingRate) {
  this->coolingRate = setCoolingRate;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro Temperatura Máxima do Simulated Annealing.
//----------------------------------------------------------------------------------------------
GASA *GASA::setMaximumTemperature(const float &maximumTemperature) {
  this->maximumTemperature = maximumTemperature;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o parâmetro Temperatura Mínima do Simulated Annealing.
//----------------------------------------------------------------------------------------------
GASA *GASA::setMinimumTemperature(const float &minimumTemperature) {
  this->minimumTemperature = minimumTemperature;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o número de ciclos de execução do AG/SA.
//----------------------------------------------------------------------------------------------
GASA *GASA::setGenerations(const int &generations) {
  this->maxGenerations = generations;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta a faixa de variação dos coeficientes dos modelos - Pesos
//----------------------------------------------------------------------------------------------
GASA *GASA::setGeneRange(GeneFormat &geneFormat, const float &min, const float &max) {
  geneFormat.min = min;
  geneFormat.max = max;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta a faixa de variação dos coeficientes dos modelos - Pesos
//----------------------------------------------------------------------------------------------
GASA *GASA::setGeneRange(const float &min, const float &max) {
  this->geneFormat.min = min;
  this->geneFormat.max = max;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta o tamanho da população.
//----------------------------------------------------------------------------------------------
GASA *GASA::setPopulationSize(const int32_t &populationSize) {
  this->populationSize = populationSize;
  return (this);
}

//----------------------------------------------------------------------------------------------
// Ajusta/reajusta os parâmetros dos algoritmos baseados em valores
// pre-determinados
//----------------------------------------------------------------------------------------------
GASA *GASA::setSAHGAParameters(const SAHGAParameter &parameters, int32_t generations,
                               int32_t population) {
  setGeneRange(-4.0, 4.0);

  maxGenerations = generations;
  populationSize = population;

  switch (parameters) {
    case SAHGAParameter::DEFAULT:
      maxIterations = 5;
      minimumTemperature = 0.001;
      maximumTemperature = 3;
      currentTemperature = maximumTemperature;
      coolingRate = 0.9;
      eliteSize = 1;
      crossoverRate = 0.8;
      mutationRate = 0.01;
      epsilon = 0.1;
      break;

    case SAHGAParameter::FAST:
      maxIterations = 3;
      minimumTemperature = 0.001;
      maximumTemperature = 3;
      currentTemperature = 3;
      coolingRate = 0.9;
      eliteSize = 1;
      crossoverRate = 0.8;
      mutationRate = 0.01;
      epsilon = 0.1;
      break;

    case SAHGAParameter::HARD:
      maxIterations = 5;
      minimumTemperature = 0.001;
      maximumTemperature = 3;
      currentTemperature = maximumTemperature;
      coolingRate = 0.9;
      eliteSize = 1;
      crossoverRate = 0.8;
      mutationRate = 0.01;
      epsilon = 0.1;
      break;

    case SAHGAParameter::ULTRA:
      maxIterations = 10;
      minimumTemperature = 0.001;
      maximumTemperature = 3;
      currentTemperature = maximumTemperature;
      coolingRate = 0.9;
      eliteSize = 1;
      crossoverRate = 0.8;
      mutationRate = 0.02;
      epsilon = 0.1;
      break;

    case SAHGAParameter::HIGHPOP:
      maxIterations = 5;
      minimumTemperature = 0.001;
      maximumTemperature = 3;
      currentTemperature = maximumTemperature;
      coolingRate = 0.75;
      eliteSize = 1;
      crossoverRate = 0.8;
      mutationRate = 0.02;
      epsilon = 0.1;
      break;
  }

  return (this);
}

//----------------------------------------------------------------------------------------------
// Construtor do Algoritmo Híbrido (AG/SA). Prepara variáveis de controle.
//----------------------------------------------------------------------------------------------
GASA::GASA(const Graph &graph, const Dataset &dataset, const ModelType &modelType,
           const ObjectiveType &objectiveFunction)
    : _random(std::make_unique<Random>(.0, 1.)) {
  this->graph = new Graph();
  this->graph->copy(graph);
  this->dataset = new Dataset();
  this->dataset->copy(dataset);
  this->modelType = modelType;
  this->objectiveFunction = objectiveFunction;
  bestChromosome.fitness = 1e100;  // Para maximizar (-1e100); Para minimizar (1e100)

  switch (modelType) {
    case ModelType::LINEAR: {  // Modelo linear --> sem vizinhança
      // #Variáveis independentes + 1 --> (#coeficientes + constante) do modelo
      geneSize = dataset.colN;
      break;
    }
    case ModelType::QUADRATIC: {  // Modelo quadrático --> sem vizinhança
      //(2 * #Variáveis independentes + 1)
      geneSize = 2 * dataset.colN - 1;
      break;
    }
    case ModelType::LAG: {  // Modelo de Regressão Espacial
      //#Variáveis independentes + 2 --> #coeficientes + constante + lambda
      geneSize = dataset.colN + 1;
      break;
    }
  }
}

//----------------------------------------------------------------------------------------------
// Destrutor do Algoritmo Híbrido (AG/SA).
//----------------------------------------------------------------------------------------------
GASA::~GASA() {
  chromosomeFormat.clear();
  population.clear();
  delete dataset;
  delete graph;
}
