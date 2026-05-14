#pragma once

#include "Encoder.hpp"
#include "Problem.hpp"
#include <string>
#include <vector>

class EdgeColoring : public Problem {
private:
    struct Triangle {
        int e1, e2, e3;
    };

    int numVertices;
    int numEdges;
    std::vector<std::pair<int, int>> edges;
    std::vector<Triangle> triangles;

    inline std::string vName(int e, int i) const {
        return "x_" + std::to_string(e) + "_" + std::to_string(i);
    }

public:
    // il costruttore riceve i nodi e la lista degli archi, e precalcola i triangoli
    EdgeColoring(int V, const std::vector<std::pair<int, int>>& E) 
        : numVertices(V), numEdges(E.size()), edges(E) {
        
        // matrice di adiacenza che mappa (u, v) all'indice dell'arco
        std::vector<std::vector<int>> adj(V, std::vector<int>(V, -1));
        for (int i = 0; i < numEdges; ++i) {
            int u = edges[i].first;
            int v = edges[i].second;
            adj[u][v] = i;
            adj[v][u] = i; // grafo non orientato
        }

        // troviamo tutti i triangolo
        for (int i = 0; i < V; ++i) {
            for (int j = i + 1; j < V; ++j) {
                if (adj[i][j] != -1) {
                    for (int k = j + 1; k < V; ++k) {
                        if (adj[i][k] != -1 && adj[j][k] != -1) {
                            triangles.push_back({adj[i][j], adj[j][k], adj[i][k]});
                        }
                    }
                }
            }
        }
    }

    void generateConstraints(Encoder& encoder, EncodingContext& ctx, 
                             int tId, int tCount) override {
        
        // ogni arco deve avere almeno un colore (1, 2 o 3)
        for (int e = tId; e < numEdges; e += tCount) {
            std::vector<int> edgeLits;
            for (int i = 1; i <= 3; ++i) {
                edgeLits.push_back(encoder.getVar(vName(e, i)));
            }
            encoder.writeDirectClause(edgeLits, ctx);
        }

        // per ogni triangolo e per ogni colore, 
        // tre archi non possono essere dello stesso colore
        // Partizionato per indice del triangolo
        for (int t = tId; t < triangles.size(); t += tCount) {
            int e1 = triangles[t].e1;
            int e2 = triangles[t].e2;
            int e3 = triangles[t].e3;

            for (int i = 1; i <= 3; ++i) {
                int lit1 = -encoder.getVar(vName(e1, i));
                int lit2 = -encoder.getVar(vName(e2, i));
                int lit3 = -encoder.getVar(vName(e3, i));
                
                encoder.writeDirectClause({lit1, lit2, lit3}, ctx);
            }
        }
    }
};
