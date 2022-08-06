#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sahga/structures/dataset.hpp>

Dataset::Dataset() {
  rowN = 0;
  colN = 0;
  stats.clear();
  M = NULL;
}

Dataset::~Dataset() {
  for (int i = 0; i < rowN; i++) free(M[i]);
  free(M);

  stats.clear();
  M = NULL;
}

/**
 * Resets array size and allocates space for data
 *
 * @param { int } row - New row length;
 * @param { int } col - New column length.
 *
 * @return { bool } true if it succeeded.
 */
bool Dataset::reset(int row, int col) {
  rowN = row;
  colN = col;
  stats.resize(colN);

  M = (double **)malloc(rowN * sizeof(double *));

  for (int i = 0; i < rowN; i++) M[i] = (double *)malloc(colN * sizeof(double));

  return (M == NULL);
}

/*
 * Copies the current matrix into src
 *
 * @param { const &Dataset } src - The new source matrix.
 *
 * @return { bool } true if it succeeded.
 * */
bool Dataset::copy(const Dataset &src) {
  if (M != NULL) {
    for (int i = 0; i < rowN; i++) free(M[i]);
    free(M);
  }

  reset(src.rowN, src.colN);
  stats = src.stats;

  for (int i = 0; i < src.rowN; ++i) memcpy(M[i], src.M[i], colN * sizeof(double));

  return (M == NULL);
}

/*
 * Update matrix status for each column
 *
 * @return { bool } true if it succeeded.
 * */
bool Dataset::updateStats() {
  Stats columnStats;

  stats.clear();

  for (int j = 0; j < colN; ++j) {
    columnStats.sum = 0;
    columnStats.sum2 = 0;

    for (int i = 0; i < rowN; ++i) {
      columnStats.sum = columnStats.sum + M[i][j];
      columnStats.sum2 = columnStats.sum2 + (M[i][j] * M[i][j]);
    }

    columnStats.avg = columnStats.sum / rowN;
    columnStats.stdDev = sqrt((1.0 / (rowN - 1))
                              * (columnStats.sum2 - (columnStats.sum * columnStats.sum) / rowN));
    stats.push_back(columnStats);
  }

  return ((int)stats.size() == colN);
}

// **TODO**: Redo this. More info on header.
/*
 *
 * Get Status
 *
 * @param { &std::vector<double> } avg - The average vector to be inserted.
 * @param { &std::vector<double> } stdDev - The standard deviation vector to be
 * inserted.
 *
 * @return { bool } true if it succeeded.
 * */
bool Dataset::getStats(const std::vector<double> &avg, const std::vector<double> &stdDev) {
  for (int i = 0; i < colN; ++i) {
    stats[i].avg = avg[i];
    stats[i].stdDev = stdDev[i];
  }

  return ((int)stats.size() == colN);
}

/*
 * Operates the columns of the matrix starting from Ci, making
 *
 * Xp = (X - Average) / Deviation
 *
 * @param { &const int } NDVar - Flag to decide if the algorithm is going to
 * normalize the dependent variable or not
 *
 * @return { (updateStats call) -> bool }
 * */
bool Dataset::normalize(const int &NDVar) {
  // Normalize the dependent variable?
  // 0 = Yes --> Normalize the initial column (Ci = 0)
  // !0 = No --> Normalize the columns starting from Ci = 1
  int Ci = (NDVar == 0) ? 0 : 1;

  for (int j = Ci; j < colN; ++j)
    for (int i = 0; i < rowN; ++i) M[i][j] = (M[i][j] - stats[j].avg) / stats[j].stdDev;

  return (updateStats());
}

int Dataset::partition(int col, int l, int r) {
  double *Temp;

  double Pivo = M[l][col];
  int i = l;
  int j = r + 1;

  while (true) {
    do {
      ++i;
    } while ((i <= r) && (M[i][col] >= Pivo));  // crescente --> <=; decrescente --> >=

    do {
      --j;
    } while (M[j][col] < Pivo);  // crescente --> >; decrescente --> <

    if (i >= j) break;
    Temp = M[i];
    M[i] = M[j];
    M[j] = Temp;
  }

  Temp = M[l];
  M[l] = M[j];
  M[j] = Temp;

  return j;
}

/*
 * Quick sort
 *
 * Sort the rows from l to r, considering each column as a key
 *
 * @param { int } col - current column
 * @param { int, int } l, r - lower and upper bound of sorting.
 * */
void Dataset::quickSort(int col, int l, int r) {
  int j;

  if (l < r) {
    j = partition(col, l, r);
    quickSort(col, l, j - 1);
    quickSort(col, j + 1, r);
  }
}
