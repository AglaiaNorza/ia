#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include "EncodingEngine.hpp"
#include "EdgeColoring.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "uso: " << argv[0] << " <file_grafo.txt>" << std::endl;
        std::cerr << "formato file: prima riga 'V E', seguita da E righe 'u v'" << std::endl;
        return 1;
    }

    std::string graphFile = argv[1];
    std::ifstream fin(graphFile);

    if (!fin) {
        std::cerr << "errore: impossibile aprire il file " << graphFile << std::endl;
        return 1;
    }

    int V, E;
    if (!(fin >> V >> E)) {
        std::cerr << "errore: file malformato." << std::endl;
        return 1;
    }

    std::vector<std::pair<int, int>> edges(E);
    for (int i = 0; i < E; ++i) {
        fin >> edges[i].first >> edges[i].second;
    }

    const int threads = std::thread::hardware_concurrency();
    std::string filename = "graph_coloring.cnf";

    std::cout << "creazione SAT encoding per grafo con " << V << " nodi e " 
              << E << " archi usando " << threads << " thread..." << std::endl;

    try {
        EncodingEngine engine(filename);
        EdgeColoring problem(V, edges);
        
        engine.run(problem, threads);

        std::cout << filename << " creato con successo." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "errore durante l'encoding: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
