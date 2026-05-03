#include <vector>
#include <thread>
#include "Encoder.hpp"
#include "Problem.hpp"

class EncodingEngine {
private:
    VarDictionary dict; // Shared across all threads
    ClauseStreamer streamer; // Manages the shared file

public:
    inline EncodingEngine(const std::string& outputPath) : streamer(outputPath) {}

    void run(Problem& problem, int numThreads) {
        Encoder encoder(dict, streamer);
        std::vector<std::jthread> workers;

        for (int i = 0; i < numThreads; ++i) {
            workers.emplace_back([&, i, numThreads]() {
                // each thread gets its own context/buffer
                EncodingContext ctx(streamer); 
                
                // we pass thread info so the problem can partition work
                problem.generateConstraints(encoder, ctx, i, numThreads);

                ctx.finalFlush(); 
            });
        }

        // jthreads automatically join when workers.clear() is called or they go out of scope
        workers.clear(); 

        // write the correct "p cnf V C" header at the start of the file
        streamer.finalize(dict.getMaxId());    
        }
};
