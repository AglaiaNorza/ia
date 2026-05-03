#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>

class VarDictionary {
private:
    // string name -> DIMACS int
    std::unordered_map<std::string, int> dict;
    
    // shared mutex allows multiple reads or one write 
    mutable std::shared_mutex dictMutex;

    // counter for the next available DIMACS ID
    // (atomic so Tseitin logic can grab new IDs without locking the map)
    std::atomic<int> nextId;

public:
    inline VarDictionary(int startId = 1) : nextId(startId) {}

    // get the id for a named variable - creates it if it doesn't exist
    inline int getOrCreate(const std::string& name) {
        // try to find variable with shared lock
        {
            std::shared_lock<std::shared_mutex> lock(dictMutex);
            auto it = dict.find(name);
            if (it != dict.end()) {
                return it->second;
            }
        }

        // if not found, insert it
        {
            std::unique_lock<std::shared_mutex> lock(dictMutex);
            
            // double-check check in case another thread inserted it while we were switching locks
            auto it = dict.find(name);
            if (it != dict.end()) return it->second;

            int id = nextId.fetch_add(1, std::memory_order_relaxed);
            dict[name] = id;
            return id;
        }
    }

    // for Tseitin: get a brand new ID that isn't named
    inline int getNextTseitinId() {
        return nextId.fetch_add(1, std::memory_order_relaxed);
    }

    // total variables used
    inline int getMaxId() const {
        return nextId.load(std::memory_order_relaxed) - 1;
    }
};
