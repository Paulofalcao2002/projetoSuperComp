#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

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

// Função para descobrir a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo,
                                  int numVertices) {
  vector<int> cliqueMaxima;
  vector<int> candidatos;

  for (int i = 0; i < numVertices; i++) {
    candidatos.push_back(i);
  }

  while (candidatos.size() != 0) {
    int v = candidatos[candidatos.size() - 1];
    candidatos.pop_back();

    bool podeAdicionar = true;

    for (auto u : cliqueMaxima) {
      // Se não está ligado a alguém da clique máxima
      // não pode adicionar
      if (grafo[u][v] == 0) {
        podeAdicionar = false;
        break;
      }
    }

    if (podeAdicionar) {
      cliqueMaxima.push_back(v);
      vector<int> novosCandidatos;

      // Busca novos candidatos que são adjacentes a todos os membros
      // da clique máxima
      for (auto u : candidatos) {
        bool adjacenteATodos = true;

        for (auto c : cliqueMaxima) {
          if (grafo[u][c] == 0) {
            adjacenteATodos = false;
            break;
          }
        }

        if (adjacenteATodos) {
          novosCandidatos.push_back(u);
        }
      }
      candidatos = novosCandidatos;
    }
  }

  return cliqueMaxima;
}

int main() {
  int numVertices;
  vector<vector<int>> grafo = lerGrafo("grafo.txt", numVertices);

  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo, numVertices);

  for (auto vertice : cliqueMaxima) {
    cout << vertice + 1 << " ";
  }

  cout << endl;

  return 0;
}