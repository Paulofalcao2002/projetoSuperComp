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
                                     int verticeAtual, vector<int> &candidatos,
                                     unordered_map<string, vector<int>> &memo) {

  // Gera uma chave para combinação de candidatos e vértice atual
  string key = to_string(verticeAtual);
  for (int candidate : candidatos) {
    key += "_" + to_string(candidate);
  }

  // Verifica se a chave está no mapa memoizado
  bool inMemo = memo.find(key) != memo.end();
  vector<int> memoValue;
  if (inMemo) {
    // Por ser um recurso compartilhado, o mapa é uma zona crítica
    #pragma omp critical
    {
      memoValue = memo[key];
    }
    return memoValue;
  }

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
        encontrarCliqueMaximaRec(grafo, novoCandidato, novosCandidatos, memo);

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

  // Adiciona a clique calculada na memoização
  #pragma omp critical
  {
    memo[key] = cliqueMaximaCandidato;
  }

  // Retorna a maior clique para aquele candidato
  return cliqueMaximaCandidato;
}

// Função principal para encontrar a clique máxima
vector<int> encontrarCliqueMaxima(const vector<vector<int>> &grafo,
                                  int numVertices) {
  // Inicializa vetor pra clique atual, maior clique e primeiro vetor de candidatos
  vector<int> cliqueAtual;
  vector<int> melhorClique;
  vector<int> candidatos;
  unordered_map<string, vector<int>> memo; // Usa um mapa para memoização
  memo.clear();

  // Preenche o primeiro vetor de candidatos, inicialmente todos os vértices são candidatos
  for (int i = 0; i < numVertices; i++) {
    candidatos.push_back(i);
  }

  // Acha a maior clique para cada candidato, e se for maior do que a maior clique, 
  // atualiza o valor da maior clique
  // Usa omp para calcular cliques em threads separadas
  #pragma omp parallel for 
  for (auto candidato : candidatos) {
    cliqueAtual = encontrarCliqueMaximaRec(grafo, candidato, candidatos, memo);

    if (cliqueAtual.size() > melhorClique.size()) {
      melhorClique = cliqueAtual;
    }
  }

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
