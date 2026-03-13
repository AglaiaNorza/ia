#include "search_core.hpp"
#include <unordered_set>
#include <iostream>

#pragma once

class GenericFrontier : public Frontier {
private:
    // "lambda" for comparison logic
    std::function<bool(Node*, Node*)> is_better;

public:
    // lambda is given to constructor
    GenericFrontier(std::function<bool(Node*, Node*)> comp_func) {
        this->is_better = comp_func;
    }

    Node* dequeue() override {
        if (is_empty()) return nullptr;
        
        // uses given function
        auto extracted = std::min_element(nodes.begin(), nodes.end(), is_better);
        
        Node* node = *extracted;

        // faster than remove(node);
        nodes.erase(extracted);
        stateIndexMap.erase(node->get_state()->to_string());

        return node;
    }

    void replace_if_better(Node* new_node) override {
        std::string state_str = new_node->get_state()->to_string();
        
        if (stateIndexMap.find(state_str) != stateIndexMap.end()) {
            Node* old_node = stateIndexMap[state_str];
            
            if (is_better(new_node, old_node)) {
                remove(old_node);
                add(new_node);
            } else {
                delete new_node;
            }
        }
    }
};


class FrontierFactory {
private:

    static bool tie_breaker(Node* a, Node* b) {
        return a->get_state()->to_string() < b->get_state()->to_string();
    }

public:

    static Frontier* create_dfs_frontier() {
        auto cmp = [](Node* a, Node* b) {
            if (a->get_depth() == b->get_depth()) return tie_breaker(a, b);
            return a->get_depth() > b->get_depth(); 
        };
        return new GenericFrontier(cmp);
    }

    static Frontier* create_bfs_frontier() {
        auto cmp = [](Node* a, Node* b) {
            if (a->get_depth() == b->get_depth()) return tie_breaker(a, b);
            return a->get_depth() < b->get_depth(); 
        };
        return new GenericFrontier(cmp);
    }

    static Frontier* create_ucs_frontier() {
        auto cmp = [](Node* a, Node* b) {
            if (a->gValue() == b->gValue()) return tie_breaker(a, b);
            return a->gValue() < b->gValue();
        };
        return new GenericFrontier(cmp);
    }

    static Frontier* create_greedy_frontier() {
        auto cmp = [](Node* a, Node* b) {
            if (a->hValue() == b->hValue()) return tie_breaker(a, b);
            return a->hValue() < b->hValue();
        };
        return new GenericFrontier(cmp);
    }

    static Frontier* create_astar_frontier() {
        auto cmp = [](Node* a, Node* b) {
            if (a->fValue() == b->fValue()) return tie_breaker(a, b);
            return a->fValue() < b->fValue();
        };
        return new GenericFrontier(cmp);
    }
};


class SearchImplementation{
protected:
    Frontier* frontier;
    std::unordered_set<std::string> explored;

public:
    SearchImplementation(Frontier* f) : frontier(f) {}
    
    virtual ~SearchImplementation() { delete frontier; }

    Node* search(Problem* problem, int depth_limit = -1) {
        
        explored.clear();
        
        // safety clean
        while (!frontier->is_empty()) {
            Node* n = frontier->dequeue();
            delete n; 
        }
        Node* root = new Node(problem->get_initial_state(), problem->get_heuristic(problem->get_initial_state()));
        frontier->add(root);

        while (!frontier->is_empty()) {
            Node* node = frontier->dequeue();

            std::cout << "\n----------------------------------------" << std::endl;
            std::cout << "Node: " << node->get_state()->to_string() << std::endl;
            
            frontier->print_frontier();
            
            std::cout << "Explored: {";
            int count = 0;
            for (const auto& state_str : explored) {
                std::cout << state_str;
                if (++count < explored.size()) std::cout << ", ";
            }
            std::cout << "}" << std::endl;

            if (problem->is_result(node->get_state())) {
                std::cout << ">>> reached goal ! <<<" << std::endl;
                return node;
            }

            // "explored" nodes (if it's not a tree)
            if (!problem->is_tree_search()) {
                explored.insert(node->get_state()->to_string());
            }
            
            // for iterative deepening search
            if (depth_limit != -1 && node->get_depth() >= depth_limit) {
                continue; 
            }

            std::vector<Action*> actions = problem->get_actions(node->get_state());

            for (Action* action : actions) {

                State* next_state = problem->action_result(node->get_state(), action);

                double step_cost = problem->step_cost(node->get_state(), action, next_state);
                
                Node* child = new Node(node, action, next_state, step_cost, problem->get_heuristic(next_state));

                bool in_explored = explored.find(next_state->to_string()) != explored.end();
                bool in_frontier = frontier->contains(next_state);

                if (!problem->is_tree_search()) {

                    if (!in_explored && !in_frontier) frontier->add(child);
                    else if (in_frontier) frontier->replace_if_better(child);
                    else delete child;

                } else frontier->add(child);
            }
        }

        return nullptr; // failure
    }
};