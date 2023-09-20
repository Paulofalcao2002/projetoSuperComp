#include<iostream>
#include<vector>
#include<fstream>
using namespace std;

// Função para ler o grafo a partir do arquivo de entrada
vector<vector<int>> lerGrafo(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    int numArestas;
    arquivo >> numVertices >> numArestas;

    vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

    for (int i = 0; i < numArestas; ++i) {
        int u, v;
        arquivo >> u >> v;
        grafo[u - 1][v - 1] = 1;
        grafo[v - 1][u - 1] = 1;  // O grafo é não direcionado
    }

    arquivo.close();

    return grafo;
}

// Função para descobrir a clique máxima
vector<vector<int>> encontrarCliqueMaxima(const vector<vector<int>>& grafo, int numVertices){
    vector<vector<int>> cliqueMaxima;
    
    for (auto v : grafo) {
        for (auto valor: v) {
            cout << valor << endl;
        }
    }

    return grafo;
}

int main() {
    vector<vector<int>> grafo; 

    vector<int> linha;

    linha.assign(3, 1);

    grafo.push_back(linha);
    grafo.push_back(linha);
    grafo.push_back(linha);
    encontrarCliqueMaxima(grafo, 0);
    
    cout << "Hello" << endl;
    return 0;
}