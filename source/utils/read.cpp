#include <sahga/utils/common.hpp>
#include <sahga/utils/read.hpp>

using namespace Utils::filemanagement;

int TextFile::ReadData(std::vector<std::string> &list, const std::string &fileName) {
  std::ifstream file;

  file.open(fileName);

  if (!file.is_open()) {
    fmt::print("Failed to open");
    file.clear();
    exit(0);
  }

  std::string line;
  while (std::getline(file, line)) {
    if ((line != "") && (line.substr(0, 2) != "//")) list.push_back(line);
  }

  file.close();
  return list.size();
}

int TextFile::ReadData(Dataset &M, const std::string &fileName, const char &separator) {
  std::fstream FIn;
  std::string line;
  unsigned int i;
  std::vector<std::string> lines;

  M.rowN = 0;
  FIn.open(fileName.c_str(), std::ios::in);

  if (!FIn.is_open()) {
    fmt::print("\nErro ao abrir o arquivo {}.", fileName);
    FIn.clear();
    exit(0);
  } else {
    getline(FIn, line);

    while (!FIn.eof()) {
      getline(FIn, line);
      if (line != "") lines.push_back(line);
    }
  }

  FIn.close();
  M.reset(lines.size(), 4);

  for (i = 0; i < lines.size(); ++i) {
    line = lines[i];
    std::vector<std::string> tokens = Utils::miscellaneous::split(line, separator);
    // Remove second position in order to don't get the species label
    tokens.erase(tokens.begin() + 1);

    // Convert to double array
    for (unsigned int j = 0; j < tokens.size(); ++j) {
      M.M[i][j] = std::stod(tokens[j]);
    }
  }

  lines.clear();
  return (M.rowN);
}

int TextFile::ReadData(Graph &G, Dataset &M, const std::string &fileName, const char &separator) {
  std::ifstream inputStream(fileName);

  if (!inputStream.is_open()) {
    fmt::print("\nErro ao abrir o arquivo {}.", fileName);
    inputStream.clear();
    exit(0);
  }

  std::string line;
  getline(inputStream, line);  // Consumes the header

  int32_t numNodes = (int)Utils::filemanagement::getNumber(line, separator);
  int32_t numVars = (int)Utils::filemanagement::getNumber(line, separator);

  M.reset(numNodes, numVars);

  int32_t i = 0;
  TNode node;
  while (getline(inputStream, line)) {
    if (line != "") {
      node.nodeId = (int32_t)Utils::filemanagement::getNumber(line, separator);
      node.nRel = (int)Utils::filemanagement::getNumber(line, separator);
      node.edge.resize(node.nRel);

      for (int32_t j = 0; j < node.nRel; ++j)
        node.edge[j].nodeId = (int)Utils::filemanagement::getNumber(line, separator);
      for (int32_t j = 0; j < node.nRel; ++j)
        node.edge[j].weight = Utils::filemanagement::getNumber(line, separator);

      G.insert(node);

      int32_t j = 0;
      while (line != "") {
        M.M[i][j] = Utils::filemanagement::getNumber(line, separator);
        ++j;
      }

      ++i;
    }
  }

  inputStream.close();
  return (G.nNodes == numNodes);
}

Strat_Read *Strat_Read::FileFormat(const std::string &fileName) {
  int pos;
  std::string extension;

  if ((pos = fileName.find(".", 2)) == -1) {
    fmt::print("Nome de arquivo sem extensão!");
    exit(0);
  } else {
    extension = Utils::miscellaneous::upperCase(fileName.substr(pos + 1, 3));

    if (extension == "TXT") {
      return (new TextFile);
    } else {
      fmt::print("Formato de arquivo desconhecido!");
      exit(0);
    }
  }
}

int ReadFile::Read(std::vector<std::string> &list, const std::string &fileName) {
  std::unique_ptr<Strat_Read> st(Strat_Read::FileFormat(fileName));
  return (st->ReadData(list, fileName));
}

int ReadFile::Read(Dataset &M, const std::string &fileName, const char &separator) {
  std::unique_ptr<Strat_Read> st(Strat_Read::FileFormat(fileName));
  return (st->ReadData(M, fileName, separator));
}

int ReadFile::Read(Graph &G, Dataset &M, const std::string &FileName, const char &separator) {
  std::unique_ptr<Strat_Read> st(Strat_Read::FileFormat(FileName));
  return (st->ReadData(G, M, FileName, separator));
}
