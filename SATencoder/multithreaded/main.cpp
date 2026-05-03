#include <iostream>
#include <thread>
#include <string> 
#include "EncodingEngine.hpp"
#include "NQueens.hpp"

int main(int argc, char* argv[]) {
    int N;
    if (argc > 1) {
        try {
            N = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "invalid input - provide an integer for N." << std::endl;
            return 1;
        }
    } else {
        std::cout << "enter the number of queens (N): ";
        if (!(std::cin >> N)) {
            std::cerr << "invalid input." << std::endl;
            return 1;
        }
    }

    const int threads = std::thread::hardware_concurrency();
    std::string filename = std::to_string(N) + "-queens.cnf";

    std::cout << "SAT encoding for " << N << "-Queens using " 
              << threads << " threads..." << std::endl;

    try {
        EncodingEngine engine(filename);
        NQueens queens(N);
        
        engine.run(queens, threads);

        std::cout << filename << "created." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
