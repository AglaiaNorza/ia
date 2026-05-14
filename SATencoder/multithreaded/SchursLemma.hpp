#pragma once

#include "Encoder.hpp"
#include "Problem.hpp"
#include <string>
#include <vector>

class SchursLemma : public Problem {
private:
    int N;

    inline std::string vName(int ball, int urn) const {
        return "X_" + std::to_string(ball) + "_" + std::to_string(urn);
    }

public:
    inline SchursLemma(int n) : N(n) {}

    void generateConstraints(Encoder& encoder, EncodingContext& ctx, 
                             int tId, int tCount) override {
        
        // esistenza e unicità
        // (ogni thread gestisce le biglie i tali che (i % tCount == tId))
        for (int i = 1 + tId; i <= N; i += tCount) {
            std::vector<int> lits;
            for (int j = 1; j <= 3; ++j) {
                lits.push_back(encoder.getVar(vName(i, j)));
            }

            // ogni biglia in almeno un'urna
            encoder.writeDirectClause(lits, ctx);

            // ogni biglia in al massimo un'urna
            encoder.writeDirectClause({-lits[0], -lits[1]}, ctx);
            encoder.writeDirectClause({-lits[0], -lits[2]}, ctx);
            encoder.writeDirectClause({-lits[1], -lits[2]}, ctx);
        }

        // vincolo di schur
        for (int x = 1 + tId; x <= N; x += tCount) {
            // assumiamo x <= y per evitare di generare due volte la
            // stessa combinazione (es. 1+2=3 e 2+1=3 generano le stesse clausole)
            for (int y = x; x + y <= N; ++y) {
                int z = x + y;
                
                for (int j = 1; j <= 3; ++j) {
                    int vx = encoder.getVar(vName(x, j));
                    int vy = encoder.getVar(vName(y, j));
                    int vz = encoder.getVar(vName(z, j));

                    encoder.writeDirectClause({-vx, -vy, -vz}, ctx);
                }
            }
        }
    }
};
