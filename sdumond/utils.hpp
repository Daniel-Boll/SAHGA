#include <fstream>
#include <string>
#include <vector>

using namespace std;

namespace utils {
  string exec(string command) {
    char buffer[128];
    string result = "";

    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) return "popen failed!";

    // read till end of process:
    while (!feof(pipe))
      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL) result += buffer;

    pclose(pipe);
    return result;
  }

  void split(string s, string delimiter, vector<int> *data) {
    auto start = 0U;
    auto end = s.find(delimiter);
    int i = 0;

    while (end != string::npos) {
      data->at(i) = stoi(s.substr(start, end - start));
      ++i;
      start = end + delimiter.length();
      end = s.find(delimiter, start);
    }
    data->at(i) = stoi(s.substr(start, end));
  }

  void split(string s, string delimiter, int *data) {
    auto start = 0U;
    auto end = s.find(delimiter);
    int i = 0;

    while (end != string::npos) {
      data[i] = stoi(s.substr(start, end - start));
      ++i;
      start = end + delimiter.length();
      end = s.find(delimiter, start);
    }
    data[i] = stoi(s.substr(start, end));
  }

  void getParameters(string paramsFile, vector<vector<int>> *params, int paramsAmount) {
    ifstream file(paramsFile);
    if (!file.is_open()) {
      cout << "\nCould not open file\n" << endl;
      throw "Could not open file";
    }

    string line;
    int i = 0;
    while (getline(file, line)) {
      int splitedParams[paramsAmount];
      split(line, "|", splitedParams);

      int n = sizeof(splitedParams) / sizeof(splitedParams[0]);

      vector<int> vect(splitedParams, splitedParams + n);
      vect.insert(vect.begin(), i);
      ++i;

      params->push_back(vect);
    }
    // cout << "All params: " << endl;
    // for(int i = 0; i < params->size(); i++) {
    //  for(int j=0; j < params->at(0).size(); j++)
    //	cout << params->at(i)[j] << " ";
    //    cout << endl;
    //}
  }

  void writeCsv(vector<vector<int>> data, string fileName, string columns) {
    ofstream outputFile;

    outputFile.open(fileName);
    outputFile << columns << endl;

    for (int i = 0; i < data.size(); i++)
      outputFile << to_string(data[i][0]) + "," + to_string(data[i][1]) + ","
                        + to_string(data[i][2])
                 << std::endl;

    outputFile.close();
  }
}  // namespace utils
