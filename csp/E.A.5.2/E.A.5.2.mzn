include "alldifferent.mzn";

int: n = 5;

array[1..n] of var 1..n: queens;

constraint alldifferent(queens);

constraint forall(i in 1..n, j in (i+1)..n)(
    abs(queens[i] - queens[j]) != abs(i - j)
);

solve satisfy;
