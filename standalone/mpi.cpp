#include <mpi.h>
#if defined(OPEN_MPI) && OPEN_MPI
#include <mpi-ext.h>
#endif

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <mpi-sdumont/utils.hpp>
#include <string>
#include <vector>

// 😥
using namespace std;

const short int inParamsAmount = 5;
const short int responseParamsAmount = 3;

void slaveActions() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int data[inParamsAmount];
  int response = 0;

  const std::string cd = utils::getRootDirectory("sahga-api-xmake");
  const std::string externalBinary
      = utils::format("{}/build/linux/x86_64/release/SAHGA", cd.c_str());

  while (true) {
    MPI_Recv(data, 5, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::string command = utils::format("{} {} {} {} {}", externalBinary.c_str(), data[0], data[1],
                                        data[2], data[3], data[4]);
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
  const std::string cd = utils::getRootDirectory("sahga-api-xmake");
  const std::string paramsFileName = utils::format("{}/assets/mpi/params.txt", cd.c_str());
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
