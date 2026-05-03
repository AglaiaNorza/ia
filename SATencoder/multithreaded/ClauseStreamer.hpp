#pragma once

#include <string>
#include <mutex>
#include <atomic>
#include <cstdio>
#include <stdexcept>

class ClauseStreamer {
private:
    FILE* fileHandle;
    std::mutex fileMutex;
    std::atomic<size_t> globalClauseCount;
    
    // reserve 128 bytes at byte 0 for the "p cnf V C" header.
    static constexpr size_t HEADER_PAD_SIZE = 128; 

public:
    // opens the file and writes the empty padding
    inline explicit ClauseStreamer(const std::string& filepath) : globalClauseCount(0) {
        fileHandle = fopen(filepath.c_str(), "w");
        if (!fileHandle) {
            throw std::runtime_error("failed to open output file: " + filepath);
        }

        std::string padding(HEADER_PAD_SIZE - 1, ' ');
        padding += '\n';
        fwrite(padding.c_str(), 1, HEADER_PAD_SIZE, fileHandle);
    }
    
    inline ~ClauseStreamer() {
        if (fileHandle) {
            fclose(fileHandle);
        }
    }

    // called by threads when their local buffer is full
    inline void flushThreadBuffer(const std::string& buffer, size_t localClauseCount) {
        if (buffer.empty()) return;

        // critical section
        {
            std::lock_guard<std::mutex> lock(fileMutex);
            fwrite(buffer.data(), 1, buffer.size(), fileHandle);
        }

        // update total count
        // fetch_add is atomic, memory_order_relaxed means we don't care about order 
        // but we do care about the final result
        globalClauseCount.fetch_add(localClauseCount, std::memory_order_relaxed);
    }

    inline void finalize(size_t numVars) {
        if (!fileHandle) return;

        // build the exact header string
        std::string header = "p cnf " + std::to_string(numVars) + " " + std::to_string(globalClauseCount.load());
        header += " \n";

        if (header.size() > HEADER_PAD_SIZE) {
            throw std::runtime_error("header exceeds padding size!");
        }

        // pad the remaining space with spaces so we don't overwrite clause data
        header.append(HEADER_PAD_SIZE - header.size(), ' ');

        // jump to byte 0 and overwrite the padding
        fseek(fileHandle, 0, SEEK_SET);
        fwrite(header.c_str(), 1, HEADER_PAD_SIZE, fileHandle);
        
        fclose(fileHandle);
        fileHandle = nullptr;
    }
    
    inline size_t getClauseCount() const {
        return globalClauseCount.load();
    }
};
