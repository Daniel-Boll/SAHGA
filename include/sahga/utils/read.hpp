#pragma once

#include <sahga/structures/dataset.hpp>
#include <sahga/structures/graph.hpp>
#include <string>
#include <vector>

class Strat_Read {
public:
  virtual int ReadData(Graph &G, Dataset &M, const std::string &FileName, const char &separator)
      = 0;
  virtual int ReadData(Dataset &M, const std::string &FileName, const char &separator) = 0;
  virtual int ReadData(std::vector<std::string> &List, const std::string &FileName) = 0;
  static Strat_Read *FileFormat(const std::string &FileName);
};

class TextFile : public Strat_Read {
public:
  int ReadData(Graph &G, Dataset &M, const std::string &FileName, const char &separator);
  int ReadData(Dataset &M, const std::string &FileName, const char &separator);
  int ReadData(std::vector<std::string> &List, const std::string &FileName);
};

class ReadFile {
public:
  static int Read(Graph &G, Dataset &M, const std::string &FileName, const char &separator);
  static int Read(Dataset &M, const std::string &FileName, const char &separator);
  static int Read(std::vector<std::string> &List, const std::string &FileName);
};
