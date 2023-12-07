#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <omp.h>
#include <mpi.h>
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

// Função recursiva para encontrar a clique máxima começando em um candidato
vector<int> encontrarCliqueMaximaRec(const vector<vector<int>> &grafo,
                                     int verticeAtual,
                                     vector<int> &candidatos) {

  vector<int> cliqueMaximaCandidato;
  cliqueMaximaCandidato.push_back(verticeAtual);

  vector<int> novosCandidatos;

  // Busca novos candidatos que são adjacentes a todos os membros
  // da clique do candidato
  for (auto u : candidatos) {
    bool adjacenteATodos = true;

    for (auto c : cliqueMaximaCandidato) {
      if (grafo[u][c] == 0) {
        adjacenteATodos = false;
        break;
      }
    }

    if (adjacenteATodos) {
      novosCandidatos.push_back(u);
    }
  }

  for (auto novoCandidato : novosCandidatos) {
    vector<int> cliqueNovoCandidato =
        encontrarCliqueMaximaRec(grafo, novoCandidato, novosCandidatos);

    bool podeAdicionar = true;

    for (auto u : cliqueNovoCandidato) {
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
                                  int numVertices, int iStart, int iEnd) {
  vector<int> cliqueAtual;
  vector<int> melhorClique;
  vector<int> candidatos(numVertices);

  for (int i = 0; i < numVertices; i++) {
    candidatos[i] = i;
  }

  #pragma omp parallel for 
  for (int i = iStart; i < iEnd; i++) {
    int candidato = candidatos[i];
    cliqueAtual = encontrarCliqueMaximaRec(grafo, candidato, candidatos);

    if (cliqueAtual.size() > melhorClique.size()) {
      melhorClique = cliqueAtual;
    }
  }

  return melhorClique;
}

int main() {
  MPI_Init(NULL, NULL);
  int rank, size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  vector<vector<int>> grafo;
  int numVertices = 0;

  if (rank == 0) {
    grafo = lerGrafo("grafo.txt", numVertices);
  }

  // Broadcast the number of vertices
  MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Broadcast the flattened 1D vector
  if (rank != 0) {
    grafo.resize(numVertices, vector<int>(numVertices));
  }
  
  // Flatten the 2D vector into a 1D array
  vector<int> flattened;
  flattened.reserve(numVertices * numVertices);
  for (const auto &row : grafo) {
    flattened.insert(flattened.end(), row.begin(), row.end());
  }

  MPI_Bcast(flattened.data(), numVertices * numVertices, MPI_INT, 0, MPI_COMM_WORLD);

  // Reconstruct the 2D vector in other processes
  if (rank != 0) {
    grafo.clear();
    for (int i = 0; i < numVertices; ++i) {
      grafo.emplace_back(flattened.begin() + i * numVertices, flattened.begin() + (i + 1) * numVertices);
    }
  }

  auto start = high_resolution_clock::now();

  int procCandidatosParaVerificar = numVertices / size;
  int iStart = rank * procCandidatosParaVerificar;
  int iEnd = iStart + procCandidatosParaVerificar;

  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo, numVertices, iStart, iEnd);

  if (rank == 0) {
      for (int i = 1; i < size; i++) {
        int tamanhoCliqueMaximaProc;
        MPI_Recv(&tamanhoCliqueMaximaProc, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        vector<int> cliqueMaximaProc(tamanhoCliqueMaximaProc);
        MPI_Recv(cliqueMaximaProc.data(), tamanhoCliqueMaximaProc, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (tamanhoCliqueMaximaProc > (int) cliqueMaxima.size()) {
          cliqueMaxima = cliqueMaximaProc;
        }
      }

    } else { 
      int tamanhoCliqueMaximaProc = cliqueMaxima.size();
      MPI_Send(&tamanhoCliqueMaximaProc, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
      MPI_Send(cliqueMaxima.data(), tamanhoCliqueMaximaProc, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

  if (rank == 0) {
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    cout << "Clique máxima: ";
    for (auto vertice : cliqueMaxima) {
      cout << vertice + 1 << " ";
    }
    cout << endl;
    cout << "Tamanho clique máxima: " << cliqueMaxima.size() << endl;
  }

  MPI_Finalize();

  return 0;
}
