include "globals.mzn"; % per global_cardinality

int: M;
int: N;
int: D;
array[1..N] of int: C;

array[1..N] of var 1..M: X;

var 2..N-1-D: P1; % primo picco
var 3..N-2: P2; % minimo locale 
var 4..N-1: P3; % secondo picco

array[1..M] of int: counts = [count(C, v) | v in 1..M];
constraint global_cardinality(X, [v | v in 1..M], counts);

% alternativamente:
% constraint sort(C) == sort(X);

constraint P1 < P2;
constraint P2 < P3;
constraint P3 = P1 + D;

constraint forall(i in 1..P1-1) (X[i] < X[i+1]);
constraint forall(i in P1..P2-1) (X[i] > X[i+1]);
constraint forall(i in P2..P3-1) (X[i] < X[i+1]);
constraint forall(i in P3..N-1) (X[i] > X[i+1]);

solve satisfy;
