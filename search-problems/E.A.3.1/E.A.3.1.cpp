#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../framework/domain.hpp"
#include "../framework/search_core.hpp"
#include "../framework/search_algos.hpp"

class GraphState : public State {
private:
    std::string name;
public:
    GraphState(std::string n) : name(n) {}
    
    bool operator==(const State& other) const override {
        const GraphState* other_state = (GraphState*)(&other);
        return other_state && name == other_state->name;
    }

    std::string to_string() const override {
        return name;
    }
};

class GraphAction : public Action {
private:
    std::string dest_node;
public:
    GraphAction(std::string dest) : dest_node(dest) {}
    
    std::string get_name() override { return "Go to " + dest_node; }
    std::string get_dest() { return dest_node; }
};

class EA31Problem : public Problem {
private:
    // Map: Node -> pair <Dest, Cost>
    std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> graph;
    // Heuristics map: Node -> h
    std::unordered_map<std::string, double> heuristics;

public:
    EA31Problem(State* init) : Problem(init, false) {
        graph["S"] = {{"A", 2.0}, {"B", 7.0}, {"D", 5.0}};
        graph["A"] = {{"B", 4.0}};
        graph["B"] = {{"C", 3.0}, {"G1", 9.0}};
        graph["C"] = {{"S", 1.0}, {"F", 2.0}, {"J", 5.0}};
        graph["D"] = {{"C", 3.0}, {"E", 3.0}, {"S", 8.0}};
        graph["E"] = {{"G2", 7.0}};
        graph["F"] = {{"D", 1.0}, {"G2", 4.0}};
        graph["J"] = {{"G1", 3.0}};
        
        heuristics["S"] = 7.0; heuristics["A"] = 0.0; heuristics["B"] = 3.0;
        heuristics["C"] = 2.0; heuristics["D"] = 4.0; heuristics["E"] = 5.0;
        heuristics["F"] = 3.0; heuristics["J"] = 1.0; heuristics["G1"] = 0.0;
        heuristics["G2"] = 0.0;
    }

    std::vector<Action*> get_actions(State* state) override {
        std::vector<Action*> actions;
        std::string current = state->to_string();
        
        if (graph.find(current) != graph.end()) {
            for (auto& edge : graph[current]) {
                actions.push_back(new GraphAction(edge.first));
            }
        }
        return actions;
    }

    State* action_result(State* state, Action* action) override {
        GraphAction* g_action = (GraphAction*)(action);
        return new GraphState(g_action->get_dest());
    }

    bool is_result(State* state) override {
        std::string current = state->to_string();
        return current == "G1" || current == "G2";
    }

    double step_cost(State* state, Action* action, State* new_state) override {
        std::string current = state->to_string();
        std::string next = new_state->to_string();
        
        for (auto& edge : graph[current]) {
            if (edge.first == next) {
                return edge.second;
            }
        }
        return 1.0; 
    }

    double get_heuristic(State* state) override {
        return heuristics[state->to_string()];
    }
};

int main() {
    GraphState* initial_state = new GraphState("S");
    
    // util
    auto print_solution = [](Node* result, const std::string& algo_name) {
        std::cout << "=== " << algo_name << " ===" << std::endl;
        if (result == nullptr) {
            std::cout << "nessuna soluzione trovata" << std::endl;
            return;
        }

        // Building the path bottom-up
        std::vector<std::string> path;
        Node* current = result;
        while (current != nullptr) {
            path.push_back(current->get_state()->to_string());
            current = current->get_parent();
        }

        std::cout << "cammino: ";
        for (int i = path.size() - 1; i >= 0; --i) {
            std::cout << path[i] << (i > 0 ? " -> " : "");
        }
        std::cout << "\ncosto totale (g): " << result->gValue() << std::endl;
        std::cout << std::endl;
    };

    // --- DFS ---
    {
        EA31Problem problem(new GraphState("S"));
        Frontier* dfs_frontier = FrontierFactory::create_dfs_frontier();
        SearchImplementation searcher(dfs_frontier);
        Node* result = searcher.search(&problem);
        print_solution(result, "Depth First (DFS)");
    }

    // --- BFS ---
    {
        EA31Problem problem(new GraphState("S"));
        Frontier* bfs_frontier = FrontierFactory::create_bfs_frontier();
        SearchImplementation searcher(bfs_frontier);
        Node* result = searcher.search(&problem);
        print_solution(result, "Breadth First (BFS)");
    }

    // --- Uniform Cost Search  ---
    {
        EA31Problem problem(new GraphState("S"));
        Frontier* ucs_frontier = FrontierFactory::create_ucs_frontier();
        SearchImplementation searcher(ucs_frontier);
        Node* result = searcher.search(&problem);
        print_solution(result, "Uniform Cost (UCS)");
    }

    // --- IDS ---
    {
        std::cout << "=== Iterative Deepening Search (IDS) ===" << std::endl;
        EA31Problem problem(new GraphState("S"));
        Node* result = nullptr;
        int depth_limit = 0;
        
        // Eseguiamo la DFS incrementando progressivamente il limite di profondità
        while (result == nullptr && depth_limit < 20) { // limite di sicurezza
            std::cout << "  [IDS - limite profondita': " << depth_limit << "]" << std::endl;
            
            Frontier* dfs_frontier = FrontierFactory::create_dfs_frontier();
            SearchImplementation searcher(dfs_frontier);
            
            result = searcher.search(&problem, depth_limit);
            
            if (result != nullptr) {
                print_solution(result, "risultato IDS");
            } else {
                depth_limit++;
            }
        }
    }

    // --- Best-First Greedy ---
    {
        EA31Problem problem(new GraphState("S"));
        Frontier* greedy_frontier = FrontierFactory::create_greedy_frontier();
        SearchImplementation searcher(greedy_frontier);
        Node* result = searcher.search(&problem);
        print_solution(result, "Best-First Greedy");
    }

    // --- A* ---
    {
        EA31Problem problem(new GraphState("S"));
        Frontier* astar_frontier = FrontierFactory::create_astar_frontier();
        SearchImplementation searcher(astar_frontier);
        Node* result = searcher.search(&problem);
        print_solution(result, "A*");
    }

    return 0;
}
