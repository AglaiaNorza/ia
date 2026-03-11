#include <unordered_set>
#include <iostream>

#include "frontiers.hpp"

class SearchImplementation{
protected:
    Frontier* frontier;
    std::unordered_set<std::string> explored;

public:
    SearchImplementation(Frontier* f) : frontier(f) {}
    
    virtual ~SearchImplementation() { delete frontier; }

    template <typename T, typename C>

    Node* search(Problem<T, C>* problem) {

        Node* root = new Node(problem->get_initial_state());

        frontier->add(root);

        while (!frontier->is_empty()) {
            Node* node = frontier->dequeue();

            std::cout << "nodo: " << node->get_state()->to_string() << std::endl;

            if (problem->is_result(node->get_state())) return node;

            // "explored" nodes (if it's not a tree)
            if (!problem->is_tree_search()) {
                explored.insert(node->get_state()->to_string());
            }

            std::vector<Action*> actions = problem->get_actions(node->get_state());

            for (Action* action : actions) {

                State* next_state = problem->action_result(node->get_state(), action);

                double step_cost = problem->step_cost(node->get_state(), action, next_state);
                
                Node* child = new Node(node, action, next_state, step_cost);

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
