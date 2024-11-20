module Codegen (
    exprToHandlerQuery
  , exprToClaim

  , queryValueToTupleEntry -- for testing only
  , foxtalkExprToPosition -- for testing only
) where

import Data.List (intercalate)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)
  , FoxtalkType (..)
  )

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