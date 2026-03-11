#include "base-framework.hpp"
#include <algorithm>
#include <functional>

class Frontier {
protected:
    std::vector<Node*> nodes;
    std::unordered_map<std::string, Node*> stateIndexMap;

public:
    virtual ~Frontier() = default;

    bool is_empty() {
        return nodes.empty();
    }

    virtual void add(Node* node) {
        nodes.push_back(node);
        stateIndexMap[node->get_state()->to_string()] = node;
    }

    // implemented by subclasses based on policy
    virtual Node* dequeue() = 0;

    bool contains(State* state) { return stateIndexMap.find(state->to_string()) != stateIndexMap.end(); }

    virtual void remove(Node* node) {
        if (node == nullptr) return;
        
        // remove from hashmap
        stateIndexMap.erase(node->get_state()->to_string());
        
        // remove from frontier
        nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
    }

    // to implement in subclasses: finds node in frontier, sees if it's better according
    // to the chosen measure, and, if so, removes the old one and adds the new one
    virtual void replace_if_better(Node* new_node);

};

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
