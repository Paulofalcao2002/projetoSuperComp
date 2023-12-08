%%writefile forca-bruta-ingenua.cpp

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace chrono;

// Função para ler o grafo a partir do arquivo de entrada
vector<vector<int>> lerGrafo(const string &nomeArquivo, int &numVertices) {
  ifstream arquivo(nomeArquivo);
  int numArestas;
  arquivo >> numVertices >> numArestas;

  vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

  for (int i = 0; i < numArestas; ++i) {
    int u, v;
    arquivo >> u >> v;
    grafo[u - 1][v - 1] = 1;
    grafo[v - 1][u - 1] = 1; // O grafo é não direcionado
  }

  arquivo.close();

  return grafo;
}

// Função para verificar se um conjunto de vértices forma uma clique
bool formaClique(const vector<vector<int>> &grafo,
                 const vector<int> &vertices) {
  for (int i = 0; i < (int)vertices.size(); ++i) {
    for (int j = i + 1; j < (int)vertices.size(); ++j) {
      if (grafo[vertices[i]][vertices[j]] == 0) {
        return false;
      }
    }
  }
  return true;
}

// Função recursiva para encontrar a clique máxima
void encontrarCliqueMaximaRec(const vector<vector<int>> &grafo,
                              vector<int> &cliqueAtual,
                              vector<int> &melhorClique, int verticeAtual) {
  if (verticeAtual == (int)grafo.size()) {
    if (cliqueAtual.size() > melhorClique.size() &&
        formaClique(grafo, cliqueAtual)) {
      melhorClique = cliqueAtual;
    }
    return;
  }

  // Tente incluir o vértice atual na clique
  cliqueAtual.push_back(verticeAtual);
  encontrarCliqueMaximaRec(grafo, cliqueAtual, melhorClique, verticeAtual + 1);
  cliqueAtual.pop_back(); // Desfaça a inclusão

  // Tente não incluir o vértice atual na clique
  encontrarCliqueMaximaRec(grafo, cliqueAtual, melhorClique, verticeAtual + 1);
}

// Função principal para encontrar a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo) {
  vector<int> cliqueAtual;
  vector<int> melhorClique;

  encontrarCliqueMaximaRec(grafo, cliqueAtual, melhorClique, 0);

  return melhorClique;
}

int main() {
  int numVertices;
  vector<vector<int>> grafo = lerGrafo("grafo.txt", numVertices);
  
  auto start = high_resolution_clock::now();

  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo);

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);

  cout << "Execution time: " << duration.count() << " milliseconds" << endl;

  cout << "Clique máxima: ";
  for (auto vertice : cliqueMaxima) {
    cout << vertice + 1 << " ";
  }
  cout << endl;
  cout << "Tamanho clique máxima: " << cliqueMaxima.size() << endl;

  return 0;
}