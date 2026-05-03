#pragma once

#include "Encoder.hpp"
#include "Problem.hpp"
#include <string>
#include <vector>

class NQueens : public Problem {
private:
    int N;

    // generate unique variable names for the dictionary
    inline std::string vName(int r, int c) const {
        return "q_" + std::to_string(r) + "_" + std::to_string(c);
    }

public:
    inline NQueens(int n) : N(n) {}

     // each thread handles a subset of rows/columns/diagonals based on its tId
    void generateConstraints(Encoder& encoder, EncodingContext& ctx, 
                             int tId, int tCount) override {
        
        // each thread handles rows r where (r % tCount == tId)
        for (int r = tId; r < N; r += tCount) {
            std::vector<int> rowLits;
            for (int c = 0; c < N; ++c) {
                rowLits.push_back(encoder.getVar(vName(r, c)));
            }

            // at least one queen per row
            encoder.writeDirectClause(rowLits, ctx);

            // at most one queen per row
            for (int i = 0; i < N; ++i) {
                for (int j = i + 1; j < N; ++j) {
                    encoder.writeDirectClause({-rowLits[i], -rowLits[j]}, ctx);
                }
            }
        }

        // partitioned by column index
        for (int c = tId; c < N; c += tCount) {
            std::vector<int> colLits;
            for (int r = 0; r < N; ++r) {
                colLits.push_back(encoder.getVar(vName(r, c)));            
            }

            // at least one queen per column
            encoder.writeDirectClause(colLits, ctx);

            // at most one queen per column
            for (int i = 0; i < N; ++i) {
                for (int j = i + 1; j < N; ++j) {
                    encoder.writeDirectClause({-colLits[i], -colLits[j]}, ctx);
                }
            }
        }
        // diagonals (and antidiagonals)
        for (int k = -(N - 1) + tId; k < N; k += tCount) {
            std::vector<int> diagLits;
            for (int r = 0; r < N; ++r) {
                int c = r - k;
                if (c >= 0 && c < N) {
                    diagLits.push_back(encoder.getVar(vName(r, c)));
                }
            }
            // at most one queen per diagonal
            if (diagLits.size() > 1) {
                for (int i = 0; i < diagLits.size(); ++i) {
                    for (int j = i + 1; j < diagLits.size(); ++j) {
                        encoder.writeDirectClause({-diagLits[i], -diagLits[j]}, ctx);
                    }
                }
            }
        }

        for (int k = tId; k <= (2 * N - 2); k += tCount) {
            std::vector<int> antiDiagLits;
            for (int r = 0; r < N; ++r) {
                int c = k - r;
                if (c >= 0 && c < N) {
                    antiDiagLits.push_back(encoder.getVar(vName(r, c)));
                }
            }
            if (antiDiagLits.size() > 1) {
                for (int i = 0; i < antiDiagLits.size(); ++i) {
                    for (int j = i + 1; j < antiDiagLits.size(); ++j) {
                        encoder.writeDirectClause({-antiDiagLits[i], -antiDiagLits[j]}, ctx);
                    }
                }
            }
        }
    }
};;
