module Codegen (
    exprToHandlerQuery
  , exprToClaim
  , foxtalkExprToLookup
  -- , genProg
  , genHandleBody

  , queryValueToTupleEntry -- for testing only
  , queryExprToLookup -- for testing only
  , foxtalkExprToPosition -- for testing only
  , queryValuePosToLookup -- for testing only
) where

import Data.List (intercalate)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , QueryExprF (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)
  , FoxtalkExprF (..)
  , FoxtalkType (..),
  )
import Data.Functor.Foldable (cata)
import Data.Maybe (mapMaybe)

type CExpr = String

----- Into Query -----
queryLitToTupleEntry :: QueryLiteral -> String
queryLitToTupleEntry (LSymbol l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LU64    l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LI64    l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LDouble l) = "{" ++ show l ++ "}"

queryValueToTupleEntry :: (QueryValue String) -> String
queryValueToTupleEntry (VLit l)             = queryLitToTupleEntry l
queryValueToTupleEntry (VVarIntro _ _)      = "TupleNoun::query()"
queryValueToTupleEntry (VVarBinding s)      = "{" ++ s ++ "}"
queryValueToTupleEntry (VVarIntroLit _ _ l) = queryLitToTupleEntry l

intoQuery :: QueryExpr String -> Maybe [CExpr]
intoQuery (EQueryTuple values) =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in return $ ["claim({{" ++ vs ++ "}})"]
intoQuery (EQueryAnd l r) =
  do
    ll <- intoQuery l
    rr <- intoQuery r
    return $ ll ++ rr
intoQuery (EQueryOr l r) =
  do
    ll <- intoQuery l
    rr <- intoQuery r
    return $ ll ++ rr

exprToHandlerQuery :: FoxtalkExpr String -> Maybe [CExpr]
exprToHandlerQuery (EWhen q _)      = intoQuery q
exprToHandlerQuery (EForAll _ q _)  = intoQuery q
exprToHandlerQuery _                = Nothing

----- Generate Claim -----
exprToClaim :: FoxtalkExpr String -> Maybe CExpr
exprToClaim (EClaim values) =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in return $ "claim({{" ++ vs ++ "}})"
exprToClaim _           = Nothing

----- GENERATE LOCALS -----
foxtalkTypeToCType :: FoxtalkType -> String
foxtalkTypeToCType TSymbol  = "std::string"
foxtalkTypeToCType TCptr    = "void*"
foxtalkTypeToCType TU64     = "uint64_t"
foxtalkTypeToCType TI64     = "int64_t"
foxtalkTypeToCType TDouble  = "double"
foxtalkTypeToCType TBytes   = "std::vector<uint8_t>"

generateLookupVar :: String -> FoxtalkType -> String -> Int -> CExpr
generateLookupVar queryResult t varName varIndex =
  "auto " ++ varName ++ " = " ++ queryResult
    ++ ".at<" ++ foxtalkTypeToCType t ++ ">(" ++ show varIndex ++ ")"

queryValuePosToLookup :: String -> QueryValue (Int, String) -> Maybe CExpr
queryValuePosToLookup _  (VLit _)                       = Nothing
queryValuePosToLookup qr (VVarIntro t (idx, name))      = Just $ generateLookupVar qr t name idx
queryValuePosToLookup _  (VVarBinding _)                = Nothing
queryValuePosToLookup qr (VVarIntroLit t (idx, name) _) = Just $ generateLookupVar qr t name idx

queryExprToLookup :: String -> QueryExpr (Int, String) -> [CExpr]
queryExprToLookup qr = cata go
  where
    -- Tbh, I'm not sure this is better... It's pretty cool, though.
    go :: QueryExprF (Int, String) [CExpr] -> [CExpr]
    go (EQueryTupleF exprs) = mapMaybe (queryValuePosToLookup qr) exprs
    go (EQueryAndF l r) = l ++ r
    go (EQueryOrF l r) = l ++ r

foxtalkExprToLookup :: String -> FoxtalkExpr String -> [CExpr]
foxtalkExprToLookup qr = go . foxtalkExprToPosition
  where
    go (EWhen q _)     = queryExprToLookup qr q
    -- TODO: Correctly do ForAll
    go (EForAll _ _ _) = undefined
    go (EClaim _)      = []

----- TO POSITON -----
queryValuesToPosition :: [QueryValue a] -> [QueryValue (Int, a)]
queryValuesToPosition = posn 0
  where
    posn :: Int -> [QueryValue a] -> [QueryValue (Int, a)]
    posn _ [] = []
    posn n (VLit l:xs)              = VLit l : posn (n + 1) xs
    posn n (VVarIntro t a:xs)       = VVarIntro t (n, a) : posn (n + 1) xs
    posn n (VVarBinding a:xs)       = VVarBinding (n, a) : posn (n + 1) xs
    posn n (VVarIntroLit t a l:xs)  = VVarIntroLit t (n, a) l : posn (n + 1) xs

queryExprToPosition :: QueryExpr a -> QueryExpr (Int, a)
queryExprToPosition (EQueryTuple vs)  = EQueryTuple $ queryValuesToPosition vs
queryExprToPosition (EQueryAnd l r)   = EQueryAnd (queryExprToPosition l) (queryExprToPosition r)
queryExprToPosition (EQueryOr l r)    = EQueryOr (queryExprToPosition l) (queryExprToPosition r)

handlerBodyLineToPosition :: HandlerBodyLine a -> HandlerBodyLine (Int, a)
handlerBodyLineToPosition (BCodeLine code) = BCodeLine code
handlerBodyLineToPosition (BFoxtalkExpr ft) = BFoxtalkExpr $ foxtalkExprToPosition ft

foxtalkExprToPosition :: FoxtalkExpr a -> FoxtalkExpr (Int, a)
foxtalkExprToPosition (EWhen q h)     = EWhen (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EForAll a q h) = EForAll (-1, a) (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EClaim vs)     = EClaim $ queryValuesToPosition vs


----- Genprog -----

genHandleBody :: Int -> FoxtalkExpr String -> [CExpr]
genHandleBody _ (EClaim _) = []
genHandleBody n (EWhen q bod) =
  let qr = "queryResults" ++ show n
  in
    [
        "void handle(const std::vector<Tuple> &" ++ qr ++ ") override"
      , "{"
    ] ++
    map ((++";") . ("  "++)) (queryExprToLookup qr (queryExprToPosition q)) ++
    ["}"]

genHandleBody _ (EForAll qr q bod) = undefined


-- genProg :: String -> [FoxtalkExpr String] -> String
-- genProg handlerName expr =
--   unlines $ genHandle expr