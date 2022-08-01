#pragma once

#include <sahga/utils/common.hpp>

class Layer {
public:
  std::string layerName;
  int32_t rowN, colN;
  double xCorner, yCorner;
  double xMax, yMax;
  double cellSize;
  double noData;
  double max, min;
  double avg, stdDev;

  Dataset *dataset;

  Layer();   // Ajusta propriedades da Layer
  ~Layer();  // Desaloca a regiao de memoria onde os dados estao armazenados
  Layer *load(const std::string &fileName);  // Le uma Layer indicada no path
  Layer *save(const std::string &fileName);  // Salva uma Layer indicada no path
  Layer *updateStat();  // Calcula a média e o desvio padrão dos dados da Layer
  Layer *setStat(const double &avg,
                 const double &stdDev);  // Seta os valores de média e desvio padrão da layer
  Layer *normalize();                    // Normaliza os valores da layer (Xp = (X - Media/Desvio)
  Layer *scale(const double &SMin,
               const double &SMax);  // Transforma os dados para a faixa [SMin..SMax]
  double getValue(double longitude,
                  double latitude);  // Retorna o valor da layer na coordenada (Long, Lat)
  int getX(double longitude);        // Retorna a coluna da layer correspondente à longitude
  int getY(double latitude);         // Retorna a linha da layer correspondente à latitude
  double getLong(int32_t x);         // Retorna a longitude no centro da coluna x
  double getLat(int32_t y);          // Retorna a loatitude no centro da linha y

private:
  void minMax();
};
