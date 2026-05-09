include "alldifferent.mzn";

int: n;
int: k;

array[1..n, 1..k] of var 1..(n*k): X;

% ho trovato array1d nei built-in (rende una matrice un array monodimensionale)
constraint alldifferent(array1d(X));

constraint forall(i in 1..n, j in 1..k-1)(
    X[i, j+1] - X[i, j] = i+1
);

solve satisfy;
