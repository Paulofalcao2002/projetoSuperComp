#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
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

// Função que retorna o candidato com maior adjacência
int encontraCandidatoSegundoHeuristica(const vector<vector<int>> &grafo,
                                     vector<int> &candidatos) {

  // Inicializa o número máximo de adjacências encontrado
  int maxAdjacencias = -1;
  // Inicializa o índice do candidato ideal
  int candidatoIdeal = -1;

  // Para todos os candidatos, busca o com maior adjacência
  for (int i = 0; i < (int) candidatos.size(); ++i) {
      int adjacencias = 0;
      for (int j = 0; j < (int) candidatos.size(); ++j) {
          // Se o candidato é adjacente a algum outro, incrementa seu nível de adjacência
          if (grafo[candidatos[i]][candidatos[j]] == 1) {
              adjacencias++;
          }
      }

      // Verifica se o número de adjacências é maior que o máximo encontrado até agora
      if (adjacencias > maxAdjacencias) {
          maxAdjacencias = adjacencias;
          candidatoIdeal = i;
      }
  }

  // Retorna o índice do candidato com mais adjacências
  return candidatoIdeal;
}

// Função recursiva para encontrar a clique máxima
vector<int> encontrarCliqueMaximaHeuristica(const vector<vector<int>> &grafo,
                                     vector<int> candidatos) {

  // Cria clique máxima
  vector<int> cliqueMaxima;

  // Enquanto ainda existirem candidatos
  while (candidatos.size() > 0) {
    // Usa a função da heurística para achar o candidato ideal
    int indiceCandidatoComMaiorAdjacencia = encontraCandidatoSegundoHeuristica(grafo, candidatos);
    int candidato = candidatos[indiceCandidatoComMaiorAdjacencia];

    // Apaga o candidato atual do vetor de candidatos
    candidatos.erase(candidatos.begin() + indiceCandidatoComMaiorAdjacencia);

    // Adiciona candidato a clique máxima
    cliqueMaxima.push_back(candidato);

    // Vamos construir um novo vetor de candidatos
    vector<int> novosCandidatos;
    for (auto candidatoAtual: candidatos) {
      bool adjacenteACliqueMaxima = true;

      for (auto membroClique: cliqueMaxima) {
        if (grafo[candidatoAtual][membroClique] == 0) {
          adjacenteACliqueMaxima = false;
          break;
        }
      }

      if (adjacenteACliqueMaxima) {
        novosCandidatos.push_back(candidatoAtual);
      }
    }

    // Atualiza os candidatos de acordo com os novos candidatos
    candidatos = novosCandidatos;
  }

  // Retorna clique máxima
  return cliqueMaxima;
}

// Função principal para encontrar a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo,
                                  int numVertices) {
  // Inicializa vetor pra maior clique e primeiro vetor de candidatos
  vector<int> melhorClique;
  vector<int> candidatos;

  // Preenche o primeiro vetor de candidatos, inicialmente todos os vértices são candidatos 
  for (int i = 0; i < numVertices; i++) {
    candidatos.push_back(i);
  }
  
  melhorClique = encontrarCliqueMaximaHeuristica(grafo, candidatos);

  // Retorna a maior clique
  return melhorClique;
}

int main() {
  // Lê grafo
  int numVertices;
  vector<vector<int>> grafo = lerGrafo("grafo.txt", numVertices);

  // Mede tempo inicial
  auto start = high_resolution_clock::now();

  // Executa a função de achar maior clique
  vector<int> cliqueMaxima = encontrarCliqueMaxima(grafo, numVertices);

  // Retém o tempo final
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

  return 0;
}
