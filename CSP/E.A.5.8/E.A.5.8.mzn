include "globals.mzn";

% --- parametri ---
int: P;
int: W;
int: G;
int: S = P div G;

% X[p, w] rappresenta il gruppo assegnato al giocatore p nella settimana w
array[1..P, 1..W] of var 1..G: X;


% C_size:
constraint forall(w in 1..W, g in 1..G) (
    count(p in 1..P)(X[p, w] == g) == S
);

% C_social:
constraint forall(p1, p2 in 1..P where p1 < p2) (
    sum(w in 1..W) (bool2int(X[p1, w] == X[p2, w])) <= 1 
    % ^sporco trucco da programmatori (sommatoria di booleani)
);


% C_sb1:
constraint forall(p in 1..P) (
    X[p, 1] = (p - 1) div S + 1
);

% C_sb2:
constraint forall(w in 2..W, g in 1..G-1) (
    min([p | p in 1..P where X[p, w] == g]) < 
    min([p | p in 1..P where X[p, w] == g + 1])
);


solve satisfy;
