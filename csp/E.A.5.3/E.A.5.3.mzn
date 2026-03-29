% v.1: generic (no assumptions, just an adj matrix)

int: N; 

% boolean matrix where true = edge
array[1..N, 1..N] of bool: E;

% variables: 0 = no edge, 1..3 are the colors
array[1..N, 1..N] of var 0..3: color;

% graph structure constraints
constraint forall(i, j in 1..N where i < j) (
    % undirected graph symmetry
    color[i, j] == color[j, i] /\ 

    % color iff edge
    (E[i, j] <-> color[i, j] > 0)
);

% no self-loops
constraint forall(i in 1..N) (color[i, i] == 0);

% triangle constraint
constraint forall(i, j, k in 1..N where i < j /\ j < k) (
    E[i, j] /\ E[j, k] /\ E[i, k] ->
        not (color[i, j] == color[j, k] /\ color[j, k] == color[i, k])
);

solve satisfy;


% v.2: 'cleaner' model, potentially slower (especially on dense graphs)

int: N; % nodes
int: M; % edges

% edges: edge i = (u[i], v[i])
array[1..M] of int: u;
array[1..M] of int: v;

% we have to assign colors to edges 
array[1..M] of var 1..3: color;

% we can check that our assumptions hold:
constraint assert(forall(i in 1..M) (u[i] < v[i]), 
    " didn't respect u[i] >= v[i]");

constraint assert(forall(i in 1..M-1) (
    u[i] < u[i+1] \/ (u[i] == u[i+1] /\ v[i] < v[i+1])
), "u, v aren't ordered");

% if our assumptions hold, we can check:
constraint forall(i, j, k in 1..M where i < j /\ j < k /\ u[i] == u[j] /\ v[i] == u[k] /\ v[j] == v[k]) 
(
    not (color[i] == color[j] /\ color[j] == color[k])
);

solve satisfy;


