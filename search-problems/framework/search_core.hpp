#include "domain.hpp"
#include <algorithm>
#include <functional>
#include <iostream>

#pragma once

class Node {
private:
    Node* parent;
    Action* action;
    State* state;

    int depth;

    double g;
    double h;

public:
    // constructor for root
    Node(State* initial_state, double heur = 0.0) {
        this->parent = nullptr;
        this->action = nullptr;
        this->state = initial_state;
        this->depth = 0;
        this->g = 0.0;
        this->h = heur;
    }

    // constructor for non-root
    Node(Node* p, Action* a, State* s, double step_cost, double heur = 0.0) {
        this->parent = p;
        this->action = a;
        this->state = s;
        this->depth = p -> get_depth() + 1;
        this->g = p -> gValue() + step_cost; 
        this->h = heur;
    }

    State* get_state() const { return state; }
    Node* get_parent() const { return parent; }
    Action* get_action() const { return action; }

    int get_depth() const { return depth; }

    bool is_equivalent(Node* n) {
        return *(this->state) == *(n->state);
    }

    double gValue() { return g; }
    double hValue() { return h; }
    double fValue() { return g + h; }

};

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
    virtual void replace_if_better(Node* new_node) = 0;

    virtual void print_frontier() const {
        std::cout << "Frontier: [";
        for (size_t i = 0; i < nodes.size(); ++i) {
            std::cout << nodes[i]->get_state()->to_string();
            if (i < nodes.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

};