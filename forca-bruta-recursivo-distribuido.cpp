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

 // Define uma clique máxima para o candidato, que inicialmente tem o valor do candidato
  vector<int> cliqueMaximaCandidato;
  cliqueMaximaCandidato.push_back(verticeAtual);

  // Define um vetor de novos candidatos
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

  // Para cada candidato que partem de do vértice atual 
  for (auto novoCandidato : novosCandidatos) {
    // Chama recursivamente a função. O retorno da chamada é a maior clique para aquele novo candidato
    vector<int> cliqueNovoCandidato =
        encontrarCliqueMaximaRec(grafo, novoCandidato, novosCandidatos);

    // Verifica se o vértice atual está na clique do novo candidato
    bool podeAdicionar = true;
    for (auto u : cliqueNovoCandidato) {
      // Se não está ligado a alguém da clique máxima
      // não pode adicionar
      if (grafo[u][verticeAtual] == 0) {
        podeAdicionar = false;
        break;
      }
    }

    // Se podemos adicionar o vértice atual, e essa clique é maior do que a maior clique que contém o 
    // vértice atual, atualizamos o valor clique máxima do vértice atual
    if (podeAdicionar &&
        cliqueNovoCandidato.size() + 1 > cliqueMaximaCandidato.size()) {
      cliqueNovoCandidato.push_back(verticeAtual);
      cliqueMaximaCandidato = cliqueNovoCandidato;
    }
  }

 // Retorna a maior clique para aquele candidato
  return cliqueMaximaCandidato;
}

// Função principal para encontrar a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo,
                                  int numVertices, int iStart, int iEnd) {
  // Inicializa vetor pra clique atual, maior clique e primeiro vetor de candidatos 
  vector<int> cliqueAtual;
  vector<int> melhorClique;
  vector<int> candidatos(numVertices);

  // Preenche o primeiro vetor de candidatos, inicialmente todos os vértices são candidatos 
  for (int i = 0; i < numVertices; i++) {
    candidatos[i] = i;
  }

  // Acha a maior clique para cada candidato, e se for maior do que a maior clique, 
  // atualiza o valor da maior clique
  // Usa omp para calcular cliques em threads separadas
  // Calcula apenas para os candidatos que o processo é responsável
  #pragma omp parallel for 
  for (int i = iStart; i < iEnd; i++) {
    int candidato = candidatos[i];
    cliqueAtual = encontrarCliqueMaximaRec(grafo, candidato, candidatos);

    if (cliqueAtual.size() > melhorClique.size()) {
      melhorClique = cliqueAtual;
    }
  }

  // Retorna a maior clique
  return melhorClique;
}

int main() {
  // Inicializa MPI
  MPI_Init(NULL, NULL);
  int rank, size;

  // Recupera rank do processo e tamanho da topologia
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Inicializa variáveis para grafo e tamanho de vértices
  vector<vector<int>> grafo;
  int numVertices = 0;

  // Processo zero executa o código de ler o grafo
  if (rank == 0) {
    grafo = lerGrafo("grafo.txt", numVertices);
  }

  // Processo zero faz broadcast do número de vértices
  MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Outros processos que não o zero redimensionam o grafo de acordo com o 
  // número de vértices
  if (rank != 0) {
    grafo.resize(numVertices, vector<int>(numVertices));
  }
  
  // Processos criam um array 1D para receber o grafo,
  // Processo zero cria o array 1D a partir do grafo
  vector<int> flattened;
  flattened.reserve(numVertices * numVertices);
  for (const auto &row : grafo) {
    flattened.insert(flattened.end(), row.begin(), row.end());
  }

  // Processo zero faz o broadcast do grafo 1D
  MPI_Bcast(flattened.data(), numVertices * numVertices, MPI_INT, 0, MPI_COMM_WORLD);

  // Outros processos que não o zero, preenchem o grafo 2D
  if (rank != 0) {
    grafo.clear();
    for (int i = 0; i < numVertices; ++i) {
      grafo.emplace_back(flattened.begin() + i * numVertices, flattened.begin() + (i + 1) * numVertices);
    }
  }

  // Pega tempo inicial
  auto start = high_resolution_clock::now();

  // Calcula os índices de candidatos que cada processo irá calcular
  int procCandidatosParaVerificar = numVertices / size;
  int iStart = rank * procCandidatosParaVerificar;
  int iEnd = iStart + procCandidatosParaVerificar;

  // Executa a função de achar maior clique
  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo, numVertices, iStart, iEnd);

  if (rank == 0) {
      // Processo principal recebe as maiores cliques que os outros processos calcularam
      // e obtém o valor da maior
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
      // Outros processos mandam para o processo principal a maior clique que foi
      // calculada
      int tamanhoCliqueMaximaProc = cliqueMaxima.size();
      MPI_Send(&tamanhoCliqueMaximaProc, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
      MPI_Send(cliqueMaxima.data(), tamanhoCliqueMaximaProc, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

  // Processo principal mostra resultados
  if (rank == 0) {
    // Obtém o tempo final
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Mostra o tempo final
    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    // Mostra qual é a clique máxima encontrada
    cout << "Clique máxima: ";
    for (auto vertice : cliqueMaxima) {
      cout << vertice + 1 << " ";
    }
    cout << endl;
    cout << "Tamanho clique máxima: " << cliqueMaxima.size() << endl;
  }

  // Finaliza MPI
  MPI_Finalize();

  return 0;
}
