#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <omp.h>
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

// Função recursiva para encontrar a clique máxima
vector<int> encontrarCliqueMaximaRec(const vector<vector<int>> &grafo,
                                     int verticeAtual,
                                     vector<int> &candidatos) {

  vector<int> cliqueMaximaCandidato;
  cliqueMaximaCandidato.push_back(verticeAtual);

  vector<int> novosCandidatos;

  // Busca novos candidatos que são adjacentes a todos os membros
  // da clique do candidato
  for (int i = 0; i < (int) candidatos.size(); i++) {
    int u = candidatos[i];
    bool adjacenteATodos = true;

    for (int j = 0; j < (int) cliqueMaximaCandidato.size(); j++) {
      int c = cliqueMaximaCandidato[j];
      if (grafo[u][c] == 0) {
        adjacenteATodos = false;
        break;
      }
    }

    if (adjacenteATodos) {
      novosCandidatos.push_back(u);
    }
  }

  // #pragma omp parallel for
  for (int i = 0; i < (int) novosCandidatos.size(); i++) {
    int novoCandidato = novosCandidatos[i];
    vector<int> cliqueNovoCandidato =
        encontrarCliqueMaximaRec(grafo, novoCandidato, novosCandidatos);

    bool podeAdicionar = true;

    for (int i = 0; i < (int) cliqueNovoCandidato.size(); i++) {
      int u = cliqueNovoCandidato[i];
      // Se não está ligado a alguém da clique máxima
      // não pode adicionar
      if (grafo[u][verticeAtual] == 0) {
        podeAdicionar = false;
        break;
      }
    }

    if (podeAdicionar &&
        cliqueNovoCandidato.size() + 1 > cliqueMaximaCandidato.size()) {
      cliqueNovoCandidato.push_back(verticeAtual);
      cliqueMaximaCandidato = cliqueNovoCandidato;
    }
  }

  return cliqueMaximaCandidato;
}

// Função principal para encontrar a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo,
                                  int numVertices) {
  vector<int> cliqueAtual;
  vector<int> melhorClique;
  vector<int> candidatos(numVertices);

  for (int i = 0; i < numVertices; i++) {
    candidatos[i] = i;
  }

  #pragma omp parallel for 
  for (int i = 0; i < (int) candidatos.size(); i++) {
    int candidato = candidatos[i];
    cliqueAtual = encontrarCliqueMaximaRec(grafo, candidato, candidatos);

    if (cliqueAtual.size() > melhorClique.size()) {
      melhorClique = cliqueAtual;
    }
  }

  return melhorClique;
}

int main() {
  int numVertices;
  vector<vector<int>> grafo = lerGrafo("grafo50.txt", numVertices);

  auto start = high_resolution_clock::now();

  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo, numVertices);

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
