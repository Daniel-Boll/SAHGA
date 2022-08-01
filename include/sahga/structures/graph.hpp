#pragma once

#include <sahga/structures/dataset.hpp>
#include <string>
#include <vector>

typedef struct {
  int nodeId;
  double weight;
} TEdge;

typedef struct {
  int nodeId;
  int nRel;              // Number of nodes to which the node is related
  std::vector<TEdge> edge;  // All node relationships (edges connected to node)
} TNode;

class Graph {
public:
  enum MPGTypes { UNDEFINED, HALFRADIUM, UMBD, UMBD2 };

  int nNodes;            // Number of nodes in the graph
  MPGTypes type;         // MPG type --> criteria used to generate the graph (MPG)
  std::vector<TNode> node;  // Set of nodes that make up the graph

  Graph();
  ~Graph();
  Graph *insert(const TNode &node);  // Insert new node in graph
  Graph *copy(const Graph &src);     // Copy graph into src
  Graph *createMPG(const Dataset &M, const double &R,
                   int32_t T);  // Analyze M and build a graph (MPG) based on T
  Graph *save(const std::string &inFileName, const double &R,
              const std::string &outFileName);  // Saves the graph (MPG) in a text file
};
