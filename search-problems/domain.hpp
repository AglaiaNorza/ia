#include <string>
#include <vector>

#pragma once

class State {

public:
    virtual ~State() = default;
    virtual bool operator==(const State& other) const = 0;

    virtual std::string to_string() const = 0;

};

class Action {
public:
    virtual ~Action() = default;

    // for plan printing
    virtual std::string get_name() { return ""; };

};

class Problem {

protected:
    State* initial_state;
    bool is_tree;

public: 
    Problem(State* init, bool tree = false) {
        this->initial_state = init;
        this->is_tree = tree;
    }

    virtual ~Problem() = default;

    State* get_initial_state() { return initial_state; }
    bool is_tree_search() { return is_tree; }

    virtual std::vector<Action*> get_actions(State* state) = 0;

    virtual State* action_result(State* state, Action* action) = 0;
    
    // either a state equality check or something else
    virtual bool is_result(State* state) = 0;

    virtual double step_cost(State* state, Action* action, State* new_state) {
        return 1.0; 
    }

    virtual double get_heuristic(State* state) { return 0.0; }
};