module Foxtalk (
  queryValueToTupleEntry -- for testing only
  , tupleObj -- for testing only
  , foxtalkExprToPosition -- for testing only
) where

import Data.List (intercalate)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)
  )

----- TO TUPLE ENTRY -----
queryLitToTupleEntry :: QueryLiteral -> String
queryLitToTupleEntry (LSymbol s) = "{" ++ show s ++ "}"

queryValueToTupleEntry :: (QueryValue String) -> String
queryValueToTupleEntry (VLit l)           = queryLitToTupleEntry l
queryValueToTupleEntry (VVarIntro _)      = "TupleValue::query()"
queryValueToTupleEntry (VVarBinding s)    = s
queryValueToTupleEntry (VVarIntroLit _ l) = queryLitToTupleEntry l

tupleObj :: String -> [QueryValue String] -> String
tupleObj name values =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in "Tuple " ++ name ++ " { " ++ vs ++ "}"


----- TO POSITON -----
queryValuesToPosition :: [QueryValue a] -> [QueryValue Int]
queryValuesToPosition = posn 0
  where
    posn :: Int -> [QueryValue a] -> [QueryValue Int]
    posn _ [] = []
    posn n (VLit l:xs)            = VLit l : posn (n + 1) xs
    posn n (VVarIntro _:xs)       = VVarIntro n : posn (n + 1) xs
    posn n (VVarBinding _:xs)     = VVarBinding n : posn (n + 1) xs
    posn n (VVarIntroLit _ l:xs)  = VVarIntroLit n l : posn (n + 1) xs

-- TODO Not sure this is what I need, yet, but I just want to see it work.
queryExprToPosition :: QueryExpr a -> QueryExpr Int
queryExprToPosition (EQueryTuple vs)  = EQueryTuple $ queryValuesToPosition vs
queryExprToPosition (EQueryAnd l r)   = EQueryAnd (queryExprToPosition l) (queryExprToPosition r)
queryExprToPosition (EQueryOr l r)    = EQueryOr (queryExprToPosition l) (queryExprToPosition r)

handlerBodyLineToPosition :: HandlerBodyLine a -> HandlerBodyLine Int
handlerBodyLineToPosition (BCodeLine code) = BCodeLine code
handlerBodyLineToPosition (BFoxtalkExpr ft) = BFoxtalkExpr $ foxtalkExprToPosition ft

foxtalkExprToPosition :: FoxtalkExpr a -> FoxtalkExpr Int
foxtalkExprToPosition (EWhen q h)     = EWhen (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EForAll _ q h) = EForAll (-1) (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EClaim vs)     = EClaim $ queryValuesToPosition vs