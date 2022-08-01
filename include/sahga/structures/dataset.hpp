#pragma once

#include <vector>

/*
 * Statistical Status structure of the Matrix
 *
 * Keeps track of:
 *   - Average (double **avg**)
 *   - Standard deviation (double **stdDev**)
 *   - Primary sum (double **sum**)
 *   - Secondary sum (double **sum2**)
 * */
typedef struct {
  double avg, stdDev, sum, sum2;
} Stats;

/*
 *
 * **Dataset Info class.**
 *
 * Advanced custom structure to store dataset info natively with operations
 * for its statistical data.
 *
 * **Public Interface**
 *
 * - rowN: Number of rows in the matrix;
 * - colN: Number of columns in the matrix;
 * */
class Dataset {
public:
  int rowN;    // Number of rows in the matrix
  int colN;    // Number of columns in the matrix
  double **M;  // Pointer to the matrix data
  std::vector<Stats> stats;

  // Sets initial matrix properties
  Dataset();
  // Deallocate the memory region where the data is stored
  ~Dataset();

  bool reset(int row = 1,
             int col = 1);        // Resets array size and allocates space for data
  bool copy(const Dataset &src);  // Copies the matrix into src

  bool updateStats();  // Update matrix status for each column
  bool getStats(       // TODO: Change to return [vector<double>, vector<double>]
      const std::vector<double> &avg,
      const std::vector<double> &stdDev);  // Returns the average and standard deviation data
  bool normalize(const int &NDVar);        // Operates the columns of the matrix, starting
                                           // from Ci, making (Xp = (X - Average)/Deviation)

  void quickSort(int col, int l,
                 int r);  // Ordena as linhas de l a r, considerando col como coluna chave

private:
  int partition(int col, int l, int r);
};
