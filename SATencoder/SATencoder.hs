type VarName = String
data Lit = Pos VarName | Neg VarName

data Expr = Literal Lit
          | And [Expr]
          | Or [Expr]
