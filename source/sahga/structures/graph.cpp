#include <stdlib.h>

#include <fstream>
#include <sahga/structures/dataset.hpp>
#include <sahga/structures/graph.hpp>
#include <sahga/utils/utils.hpp>
#include <vector>

Graph::Graph() {
  node.clear();
  nNodes = 0;
  type = Graph::UNDEFINED;
}

Graph::~Graph() {
  int i;

  for (i = 0; i < nNodes; ++i) node[i].edge.clear();
  node.clear();
  nNodes = 0;
}

Graph *Graph::insert(const TNode &node) {
  this->node.emplace_back(node);
  ++nNodes;

  return this;
}

// CHECK:
// Probably a deep copy should be good enough
Graph *Graph::copy(const Graph &src) {
  nNodes = src.nNodes;
  type = src.type;
  node = src.node;

  return this;
}

Graph *Graph::createMPG(const Dataset &M, const double &R, int T) {
  int i, j;
  double dist;
  TNode node;
  TEdge edge;

  type = (MPGTypes)T;
  // Para cada ponto (linha da matriz)
  for (i = 0; i < M.rowN; ++i) {
    node.edge.clear();
    node.nodeId = (int)M.M[i][0];  // Seta o #id do n�
    node.nRel = 1;                 // O n� est� relacionado consigo mesmo
    edge.nodeId = node.nodeId;     // Seta numa aresta o #id de seu auto-relacionamento
    edge.weight = 1;               // Seta nesta mesma aresta o peso de seu
                                   // auto-relacionamento
    node.edge.push_back(edge);     // Insere a aresta na lista de arestas do n�

    // Para cada ponto (linha da matriz)
    for (j = 0; j < M.rowN; ++j) {
      // Comparando dois pontos diferentes e do mesmo tipo (presença ou
      // ausência)
      if ((i != j) && (M.M[i][3] == M.M[j][3])) {
        dist = Utils::miscellaneous::distance(M.M[i][1], M.M[i][2], M.M[j][1], M.M[j][2]);

        switch (type) {
          case UNDEFINED: {
            break;
          }
          case HALFRADIUM: {
            // Se a distância entre os dois pontos for at� (1/2 raio)
            if (dist <= (R / 2)) {
              ++node.nRel;                   // Incrementa o contador de relacionamentos do n�
              edge.nodeId = (int)M.M[j][0];  // Seta numa aresta o #id do n� j
                                             // relacionado ao n� i
              edge.weight = 1;               // Seta nesta mesma aresta o peso do
                                             // relacionamento entre os n�s i e j (1)
              node.edge.push_back(edge);     // Insere a aresta na lista de arestas do n�
            } else {
              if (dist <= R) {  // Se a distância estiver estiver entre (1/2
                                // aio) e raio o peso ser� (0.5)
                ++node.nRel;
                edge.nodeId = (int)M.M[j][0];
                edge.weight = 0.5;
                node.edge.push_back(edge);
              }
            }
            break;
          }
          case UMBD: {
            if (dist <= R) {
              ++node.nRel;
              edge.nodeId = (int)M.M[j][0];
              edge.weight = 1.0 / dist;
              node.edge.push_back(edge);
            }
            break;
          }
          case UMBD2: {
            if (dist <= R) {  // Se dist�ncia menor que Raio o peso ser� cal
                              // ulado por 1/(d^2)
              ++node.nRel;
              edge.nodeId = (int)M.M[j][0];
              edge.weight = 1.0 / (dist * dist);
              node.edge.push_back(edge);
            }
            break;
          }
        }
      }
    }
    insert(node);
  }

  return this;
}

Graph *Graph::save(const std::string &inFileName, const double &R, const std::string &outFileName) {
  int i, j;
  std::fstream outFile;

  outFile.open(outFileName.c_str(), std::fstream::out);
  outFile << "//Matriz de proximidade para o arquivo: " << inFileName << " - raio = " << R
          << " km - Tipo = ";

  switch (type) {
    case UNDEFINED:
      break;
    case HALFRADIUM:
      outFile << "R/2" << std::endl;
      break;
    case UMBD:
      outFile << "1/d" << std::endl;
      break;
    case UMBD2:
      outFile << "1/(d^2)" << std::endl;
      break;
  }

  outFile << "//Formato da MPG --> #id;n;Rel1;Rel2;...;Reln;W1;W2;...;Wn" << std::endl;

  for (i = 0; i < nNodes; ++i) {
    outFile << node[i].nodeId << ';' << node[i].nRel << ';';
    for (j = 0; j < node[i].nRel; ++j) outFile << node[i].edge[j].nodeId << ';';
    for (j = 0; j < node[i].nRel; ++j) outFile << node[i].edge[j].weight << ';';
    outFile << std::endl;
  }

  outFile.close();

  return this;
}
