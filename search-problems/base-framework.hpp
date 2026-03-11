/* 
Il diagramma dovrà modellare concetti astratti come Problem, State, Node, Action (che potranno essere istanziati dall'utente tramite sottoclassi concrete), e il concetto Frontier (con le diverse politiche di gestione). Una opportuna operazione dovrà modellare l'algoritmo generale di risoluzione (che sfrutterà la Frontier definita dall'utente per il suo problema). 
Potrebbe essere utile permettere all'utente (che definisce una sottoclasse di Problem) di specificare alcune caratteristiche note dello spazio degli stati, se queste possono essere utili all'algoritmo generico (ad es., il fatto che il modello di transizione è in realtà un albero, cosa che permetterebbe di evitare di ricordare gli stati visitati).
*/

#include <string>
#include <vector>

using namespace std;

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
    virtual string get_name();

};

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
    Node(State* initial_state) {
        this->parent = nullptr;
        this->action = nullptr;
        this->state = initial_state;
        this->depth = 0;
        this->g = 0.0;
        this->h = 0.0;
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

template <typename T, typename C>
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

    virtual vector<Action*> get_actions(State* state) = 0;

    virtual State* action_result(State* state, Action* action) = 0;
    
    // either a state equality check or something else
    bool is_result(State* state) = 0;

    virtual double step_cost(State* state, Action* action, State* new_state) {
        return 1.0; 
    }

};


