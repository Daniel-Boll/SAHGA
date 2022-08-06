#include <sahga/structures/layer.hpp>

Layer::Layer()
    : layerName(""),
      rowN(0),
      colN(0),
      xCorner(0),
      yCorner(0),
      cellSize(0),
      noData(0),
      min(0),
      max(0),
      avg(0),
      stdDev(0),
      dataset(nullptr) {}

Layer::~Layer() { delete dataset; }

Layer *Layer::load(const std::string &fileName) {
  std::ifstream inputStream;

  inputStream.open(fileName.c_str());

  if (!inputStream.is_open()) {
    // fmt::print("Erro ao abrir o arquivo {}.\n", fileName);
    inputStream.clear();
    exit(0);
  } else {
    std::string line;
    getline(inputStream, line);

    using namespace Utils::filemanagement;

    while ((line != "") && (isalpha(line[0]))) {
      std::string subString = splitOnce(line, ' ');

      if (subString == "NCOLS") colN = (int32_t)getNumber(line, ' ');
      if (subString == "NROWS") rowN = (int32_t)getNumber(line, ' ');
      if (subString == "XLLCORNER") xCorner = getNumber(line, ' ');
      if (subString == "YLLCORNER") yCorner = getNumber(line, ' ');
      if (subString == "CELLSIZE") cellSize = getNumber(line, ' ');
      if (subString == "NODATA_VALUE") noData = getNumber(line, ' ');

      getline(inputStream, line);
    }

    xMax = xCorner + (colN - 1) * cellSize;
    yMax = yCorner + (rowN - 1) * cellSize;

    dataset = new Dataset();
    dataset->reset(rowN, colN);

    int32_t i = 0;
    while (!inputStream.eof()) {
      if (line != "") {
        for (int32_t j = 0; j < colN; ++j) dataset->M[i][j] = getNumber(line, ' ');
        ++i;
      }
      getline(inputStream, line);
    }

    minMax();
  }

  return this;
}

Layer *Layer::save(const std::string &fileName) {
  std::ofstream outputStream;

  outputStream.open(fileName.c_str());
  outputStream << std::setiosflags(std::ios::fixed);
  outputStream << "NCOLS " << std::setprecision(0) << colN << "\n";
  outputStream << "NROWS " << rowN << "\n";
  outputStream << "XLLCORNER " << std::setprecision(6) << xCorner << "\n";
  outputStream << "YLLCORNER " << yCorner << "\n";
  outputStream << "CELLSIZE " << cellSize << "\n";
  outputStream << "NODATA_VALUE " << std::setprecision(0) << noData << "\n";

  for (int32_t i = 0; i < rowN; ++i) {
    for (int32_t j = 0; j < colN; ++j) outputStream << dataset->M[i][j] << ' ';
    outputStream << "\n";
  }

  outputStream.close();

  return this;
}

Layer *Layer::updateStat() {
  double sum = 0;
  double sum2 = 0;

  int32_t N = 0;
  for (int32_t i = 0; i < rowN; ++i) {
    for (int32_t j = 0; j < colN; ++j) {
      if (dataset->M[i][j] != noData) {
        ++N;
        sum = sum + dataset->M[i][j];
        sum2 = sum2 + (dataset->M[i][j] * dataset->M[i][j]);
      }
    }
  }

  avg = sum / N;
  stdDev = sqrt((1.0 / (N - 1)) * (sum2 - (sum * sum) / N));

  return this;
}

Layer *Layer::setStat(const double &avg, const double &stdDev) {
  this->avg = avg;
  this->stdDev = stdDev;

  return this;
}

Layer *Layer::normalize() {
  for (int32_t i = 0; i < rowN; ++i)
    for (int32_t j = 0; j < colN; ++j)
      if (dataset->M[i][j] != noData) dataset->M[i][j] = (dataset->M[i][j] - avg) / stdDev;

  minMax();
  return this;
}

Layer *Layer::scale(const double &SMin, const double &SMax) {
  minMax();

  for (int32_t i = 0; i < rowN; ++i)
    for (int32_t j = 0; j < colN; ++j)
      if (dataset->M[i][j] != noData)
        dataset->M[i][j]
            = floor(((dataset->M[i][j] - min) / (max - min)) * (SMax - SMin) + SMin + 0.5);

  return this;
}

double Layer::getValue(double longitude, double latitude) {
  if ((longitude < xCorner) || (longitude > xMax) || (latitude < yCorner) || (latitude > yMax))
    return (noData);

  int32_t x = (int32_t)(fabs(longitude - xCorner) / cellSize);
  int32_t y = (int32_t)(rowN - (fabs(latitude - yCorner) / cellSize));

  return (dataset->M[y][x]);
}

int Layer::getX(double longitude) { return (int)(fabs(longitude - xCorner) / cellSize); }

int Layer::getY(double latitude) { return (int)(rowN - (fabs(latitude - yCorner) / cellSize)); }

double Layer::getLong(int x) { return (xCorner + x * cellSize); }

double Layer::getLat(int y) { return (yCorner + (rowN - 1 - y) * cellSize); }

void Layer::minMax() {
  min = std::numeric_limits<double>::min();
  max = std::numeric_limits<double>::max();

  for (int32_t i = 0; i < rowN; ++i)
    for (int32_t j = 0; j < colN; ++j)
      if (dataset->M[i][j] != noData) {
        if (min > dataset->M[i][j]) min = dataset->M[i][j];
        if (max < dataset->M[i][j]) max = dataset->M[i][j];
      }
}
