#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>

#include "utils.hpp"

using namespace std;

// use this for local machine
const string pathToExternal
    = "../build/linux/x86_64/release/SAHGA ";  // pq não posso concatenar com + " "
const string pathToParamsFile = "./params.txt";

// use this for sdumont
// const string pathToExternal = "/scratch/ppar_unioeste/felipi.matozinho/sdumond/external ";
// const string pathToExternal
//     = "/scratch/ppar_unioeste/felipi.matozinho/sahga-api-xmake/build/linux/x86_64/release/SAHGA
//     ";
// const string pathToParamsFile
//     = "/scratch/ppar_unioeste/felipi.matozinho/sahga-api-xmake/sdumond/params.txt";

const short int inParamsAmount = 5;
const short int responseParamsAmount = 3;

void slaveActions() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int data[inParamsAmount];
  int response = 0;

  while (true) {
    MPI_Recv(data, 5, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    string command = pathToExternal + to_string(data[0]) + " " + to_string(data[1]) + " "
                     + to_string(data[2]) + " " + to_string(data[3]) + " " + to_string(data[4]);
    system(command.c_str());

    MPI_Send(&response, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}

void masterAction(int numProcesses, string paramsFileName) {
  // get data from the file
  vector<vector<int>> params;
  utils::getParameters(paramsFileName, &params, inParamsAmount - 1);

  int freeRank = -1;
  bool dataEnded = false;
  unsigned int dataToSendLenght = params.size();
  int dataToSendIdx = 0;

#pragma omp parallel num_threads(2) \
    firstprivate(numProcesses, dataEnded, dataToSendLenght, params) shared(freeRank)
  {
#pragma omp sections nowait
    {
      // section to send data
#pragma omp section
      {
        cout << "At section" << endl;
        for (int i = 1; i < numProcesses; i++) {
          MPI_Send(&params[dataToSendIdx][0], inParamsAmount, MPI_INT, i, 0, MPI_COMM_WORLD);
          dataToSendIdx++;
        }

        while (!dataEnded) {
          if (freeRank == -1) continue;
          MPI_Send(&params[dataToSendIdx][0], inParamsAmount, MPI_INT, freeRank, 0, MPI_COMM_WORLD);

#pragma omp critical(freeRankModify)
          freeRank = -1;

          if (dataToSendIdx == dataToSendLenght - 1) dataEnded = true;

          dataToSendIdx++;
        }
      }

      // section to receive data
#pragma omp section
      {
        MPI_Status status;
        int receivedDataAmount = 0;
        int receivedData;

        while (receivedDataAmount < dataToSendLenght) {
          MPI_Recv(&receivedData, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

#pragma omp critical(freeRankModify)
          freeRank = status.MPI_SOURCE;

          receivedDataAmount++;
        }
      }
    }
  }
  MPI_Abort(MPI_COMM_WORLD, 0);
}

int main(int argc, char **argv) {
  string paramsFileName = pathToParamsFile;
  int provided;
  // MPI_Init(&argc, &argv);
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  int rank;
  int size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Garante que teremos pelo menos 2 processos
  if (size < 2) MPI_Abort(MPI_COMM_WORLD, 1);

  if (rank == 0)
    masterAction(size, paramsFileName);
  else
    slaveActions();

  MPI_Finalize();
}
