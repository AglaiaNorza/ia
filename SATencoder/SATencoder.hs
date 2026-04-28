import Control.Monad.Trans.State.Strict
import qualified Data.Map.Strict as Map
-- maybe consider importing unordered-containers & using Data.Hashmap.Strict
import qualified Data.Set as Set
import Data.Map.Strict ((!)) -- bang !!!
import qualified Data.ByteString.Builder as B
import System.IO (withFile, IOMode(WriteMode))
import Data.Foldable (foldMap')
import Data.List (tails)

import System.CPUTime
import Text.Printf
import System.Environment (getArgs)

type VarName = String
type VarMap = Map.Map VarName Int

data Expr = Var VarName | Not Expr | And [Expr] | Or [Expr] deriving (Show, Eq)

-- we use the State monad to keep track of the id
-- (while running this function, haskell keeps a 'hidden state' of a tuple:
-- an int, the id counter, and a VarMap, the dictionary)
-- we don't return anything because this is a side-effect-only function
buildMapDict :: Expr -> State (Int, VarMap) ()
buildMapDict expr = case expr of
    Var name -> do
        (nextId, currentMap) <- get -- get gets the state
        case Map.lookup name currentMap of
            Just _ -> return () -- already in the dictionary
            Nothing -> put (nextId +1, Map.insert name nextId currentMap)
            -- we assign the current variable to nextId and put the next-next id in the State

    Not e -> buildMapDict e

    -- map just applies the function to the list (doesn't keep track of state changes),
    -- mapM keeps track of the state changes and the results of the function 
    -- mapM_ discards the results (here, we just return (), so we don't need them)
    -- this way, it runs buildMapDict on every element of exprs and updates the state (in cascade)
    And exprs -> mapM_ buildMapDict exprs -- exprs is a list of sub-exprs 
    Or exprs  -> mapM_ buildMapDict exprs

-- i have a lot of fun with point-free style ngl
createMapping :: Expr -> VarMap
createMapping = snd . flip execState (1, Map.empty) . buildMapDict

data EncodeState = EncodeState { nextNewId::Int, cnfClauses::[[Int]], clauseCount::Int }

-- get next Id
getNextId :: State EncodeState Int
getNextId = do
    st <- get
    let nextId = nextNewId st
    put (st { nextNewId = nextId + 1 }) -- syntax to update a record
    -- creates a copy of st with the cnfClauses being the same but nextId+1
    -- equivalent to put (EncodeState { nextFreshId = fresh + 1, cnfClauses = cnfClauses st })
    return nextId

-- add newly found clauses
addClauses :: [[Int]] -> State EncodeState ()
addClauses newCs = do
    st <- get
    -- the ++ here is linear wrt newCS (which is like 3/4 clauses-long)
    -- (we're almost doing a : )
    put (st { cnfClauses = newCs ++ cnfClauses st, clauseCount = clauseCount st + length newCs })

-- tseitin: we want a set of clauses that enforces NEW <-> C1 AND C2 AND C3
-- for the AND case, we have:
-- NEW -> C1, NEW -> C2, NEW -> C3
-- so we do -NEW OR C1 etc
-- we also need C1 AND C2 .. -> NEW
-- so we do one big -C1, -C2, NEW clause
-- same reasoning for OR

encodeNode :: VarMap -> Expr -> State EncodeState Int
encodeNode dict expr = case expr of
    -- we know for sure that the var is in the dict, so we can afford to BANG!
    Var name -> return (dict ! name)

    -- equivalent to:
    -- do childId <- encodeNode dict e + return (-childId)
    -- and/or Not e -> fmap negate (encodeNode dict e) 
    Not e -> negate <$> encodeNode dict e

    And exprs -> do
        -- get IDs for children
        childIds <- mapM (encodeNode dict) exprs
        -- get ID for this new tseitsin 'element'
        fresh <- getNextId
        addClauses (foldr (\c acc -> -c : acc) [fresh] childIds : map (\c -> [-fresh, c]) childIds)
        return fresh

    Or exprs -> do
        childIds <- mapM (encodeNode dict) exprs
        fresh <- getNextId
        addClauses (foldr (:) [-fresh] childIds : map (\c -> [-c, fresh]) childIds)
        return fresh

type Clause = [Int]

encodeDirect :: VarMap -> Expr -> [Clause]
encodeDirect dict expr = go expr []
  where
    go (And exprs) acc = foldr go acc exprs
    -- use collectLits to flatten any nested Or structures within a clause
    go (Or exprs)  acc = collectLits exprs : acc 
    go v@(Var _)   acc = [litToInt dict v] : acc
    go n@(Not _)   acc = [litToInt dict n] : acc

    -- recursively finds all literals inside nested Or structures
    collectLits [] = []
    collectLits (Or es : rest) = collectLits es ++ collectLits rest
    collectLits (l : rest)     = litToInt dict l : collectLits rest

-- force inlining for the literal conversion
litToInt :: Map.Map String Int -> Expr -> Int
litToInt dict (Var name) = dict!name
litToInt dict (Not (Var name)) = negate (dict!name)
litToInt _ _ = error "Invalid literal"


toNNF :: Expr -> Expr
toNNF (Not (Var n)) = Not (Var n)
toNNF (Not (Not e)) = toNNF e
toNNF (Not (And es)) = Or  (map (toNNF . Not) es)
toNNF (Not (Or es)) = And (map (toNNF . Not) es)
toNNF (And es) = And (map toNNF es)
toNNF (Or es) = Or  (map toNNF es)
toNNF v@(Var _) = v

distribute :: Expr -> Expr -> Expr
distribute (And es) f = And [distribute e f | e <- es]
distribute e (And fs) = And [distribute e f | f <- fs]
distribute e f        = Or [e, f]

toCNF :: Expr -> Expr
toCNF (And es) = And (map toCNF es)
toCNF (Or []) = Or []
toCNF (Or [e]) = toCNF e
toCNF (Or (e:es)) = distribute (toCNF e) (toCNF (Or es))
toCNF v@(Var _) = v
toCNF n@(Not (Var _)) = n
toCNF _ = error "formula must be in NNF before calling toCNF"

-- string builders are faster.... (though very ugly)
-- B.intDec turns an int into a Builder
-- B.char7 writes a single ASCII character
-- B.string7 writes an ASCII string
-- <> = fast "append" for Builders
formatClause :: [Int] -> B.Builder
formatClause [] = B.string7 "0\n"
formatClause (x:xs) = B.intDec x <> B.char7 ' ' <> formatClause xs

formatHeader :: Int -> Int -> B.Builder
formatHeader numVars numClauses =
    B.string7 "p cnf " <> B.intDec numVars <> B.char7 ' ' <> B.intDec numClauses <> B.char7 '\n'


writeDIMACS :: FilePath -> EncodeState -> IO ()
writeDIMACS filepath finalState = do
    let numVars = nextNewId finalState - 1
        clauses = cnfClauses finalState
        numClauses = clauseCount finalState
        
        fullBuilder = formatHeader numVars numClauses <> mconcat (map formatClause clauses)

    withFile filepath WriteMode $ \handle ->
        B.hPutBuilder handle fullBuilder
        
    putStrLn $ "wrote " ++ show numClauses ++ " clauses to " ++ filepath

-- measure execution time
timeIt :: IO a -> IO (a, Double)
timeIt action = do
    start <- getCPUTime
    result <- action
    end <- getCPUTime
    let diff = fromIntegral (end - start) / (10^12)
    return (result, diff)

runEncoder :: String -> Int -> FilePath -> IO ()
runEncoder mode n outputPath = do
    printf "encoding %d-Queens (mode: %s)...\n" n mode
    
    ((finalState, numVars), duration) <- timeIt $ do
        let formula = nQueens n
        let dict = createMapping formula
        let highestId = Map.size dict
        
        case mode of
            "--tseitin" -> do
                let initialState = EncodeState highestId [] 0
                let (rootId, st) = runState (encodeNode dict formula) initialState
                let finalSt = st { cnfClauses = [rootId] : cnfClauses st, clauseCount = clauseCount st + 1 }
                let !count = length (cnfClauses finalSt)
                return (finalSt, nextNewId finalSt - 1)

            "--cnf" -> do
                let clauses = encodeDirect dict formula
                let finalSt = EncodeState highestId clauses (length clauses)
                let !count = length (cnfClauses finalSt)
                return (finalSt, highestId)

            "--direct" -> do
                let safeFormula = toCNF (toNNF formula)
                let clauses = encodeDirect dict safeFormula
                let finalSt = EncodeState highestId clauses (length clauses)
                let !count = length (cnfClauses finalSt)
                return (finalSt, highestId)
                
            _ -> error "Unknown mode: use --tseitin, --direct, or --cnf"

    printf "variables: %d\n" numVars
    printf "clauses: %d\n" (clauseCount finalState)
    printf "time: %.4fs\n" duration
    writeDIMACS outputPath finalState


main :: IO ()
main = do
    args <- getArgs
    case args of
        [mode, nStr, out] | mode `elem` ["--tseitin", "--direct", "--cnf"] -> 
            runEncoder mode (read nStr) out
        _ -> do
            putStrLn "usage: ./SATencoder [mode] <N> <output_file>"
            putStrLn "modes: --tseitin (linear, adds vars), --direct (exp clauses, no new vars), --cnf (assume input is CNF)"
