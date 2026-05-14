#include <iostream>
#include <thread>
#include <string> 
#include "EncodingEngine.hpp"
#include "SchursLemma.hpp"

int main(int argc, char* argv[]) {
    int N;
    if (argc > 1) {
        try {
            N = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "input non valido - fornisci un intero per N" << std::endl;
            return 1;
        }
    } else {
        std::cout << "inserisci il numero di biglie (N): ";
        if (!(std::cin >> N)) {
            std::cerr << "input non valido." << std::endl;
            return 1;
        }
    }

    const int threads = std::thread::hardware_concurrency();
    std::string filename = std::to_string(N) + "-schur.cnf";

    std::cout << "codifica SAT per il Lemma di Schur con " << N << " biglie usando " 
              << threads << " threads..." << std::endl;

    try {
        EncodingEngine engine(filename);
        SchursLemma schur(N);
        
        engine.run(schur, threads);

        std::cout << "file " << filename << " creato." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "errore: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
