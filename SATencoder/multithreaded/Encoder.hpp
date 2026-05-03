#pragma once

#include "VarDictionary.hpp"
#include "ClauseStreamer.hpp"
#include "AST.hpp"
#include <vector>
#include <string>

// structure to manage thread-local clause buffering
struct EncodingContext {
    std::string buffer;
    size_t clauseCount = 0;
    ClauseStreamer& streamer;
    
    // threshold for flushing (1MB)
    static constexpr size_t FLUSH_THRESHOLD = 1024 * 1024;

    inline EncodingContext(ClauseStreamer& s) : streamer(s) {
        buffer.reserve(FLUSH_THRESHOLD + 1024); 
    }

    // adds a clause string and flushes to the streamer if the buffer is full
    inline void addClauseString(const std::string& line) {
        buffer += line;
        clauseCount++;
        if (buffer.size() >= FLUSH_THRESHOLD) {
            streamer.flushThreadBuffer(buffer, clauseCount);
            buffer.clear();
            clauseCount = 0;
        }
    }

    // ensures any remaining clauses are written to the file when the thread finishes
    inline void finalFlush() {
        if (!buffer.empty()) {
            streamer.flushThreadBuffer(buffer, clauseCount);
            buffer.clear();
            clauseCount = 0;
        }
    }
};

class Encoder {
private:
    VarDictionary& dict;

public:
    inline Encoder(VarDictionary& d, ClauseStreamer& s) : dict(d) {}

    inline int getVar(const std::string& name) {
        return dict.getOrCreate(name);
    }

    // CNF vers: writes a vector of literals directly.
    inline void writeDirectClause(const std::vector<int>& clause, EncodingContext& ctx) {
        std::string line;
        for (int lit : clause) line += std::to_string(lit) + " ";
        line += "0\n";
        ctx.addClauseString(line);
    }

     // tseitin Version: recursive traversal of the AST
    inline int encodeTseitin(const Expr& expr, EncodingContext& ctx) {
        return std::visit([&](auto&& node) -> int {
            using T = std::decay_t<decltype(node)>;

            // base case: variable
            if constexpr (std::is_same_v<T, VarNode>) {
                return dict.getOrCreate(node.name);
            }

            // not
            else if constexpr (std::is_same_v<T, std::unique_ptr<NotNode>>) {
                return -encodeTseitin(node->child, ctx);
            }

            // AND
            else if constexpr (std::is_same_v<T, std::unique_ptr<AndNode>>) {
                int fresh = dict.getNextTseitinId();
                std::vector<int> childrenIds;
                for (auto& child : node->children) {
                    childrenIds.push_back(encodeTseitin(child, ctx));
                }

                // (fresh -> ci) => (-fresh v ci)
                for (int cid : childrenIds) {
                    ctx.addClauseString(std::to_string(-fresh) + " " + std::to_string(cid) + " 0\n");
                }
                
                // (c1 & c2 & ... -> fresh) => (-c1 v -c2 v ... v fresh)
                std::string longClause;
                for (int cid : childrenIds) {
                    longClause += std::to_string(-cid) + " ";
                }
                longClause += std::to_string(fresh) + " 0\n";
                ctx.addClauseString(longClause);

                return fresh;
            }

            // OR
            else if constexpr (std::is_same_v<T, std::unique_ptr<OrNode>>) {
                int fresh = dict.getNextTseitinId();
                std::vector<int> childrenIds;
                for (auto& child : node->children) {
                    childrenIds.push_back(encodeTseitin(child, ctx));
                }

                // (ci -> fresh) => (-ci v fresh)
                for (int cid : childrenIds) {
                    ctx.addClauseString(std::to_string(-cid) + " " + std::to_string(fresh) + " 0\n");
                }

                // (fresh -> c1 | c2 | ...) => (-fresh v c1 v c2 v ...)
                std::string longClause = std::to_string(-fresh) + " ";
                for (int cid : childrenIds) {
                    longClause += std::to_string(cid) + " ";
                }
                longClause += "0\n";
                ctx.addClauseString(longClause);

                return fresh;
            }
        }, expr);
    }
};
