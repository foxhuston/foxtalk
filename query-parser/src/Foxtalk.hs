module Foxtalk (
  queryValueToTupleEntry -- for testing only
  , tupleObj -- for testing only
) where

import Data.List (intercalate)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  -- , QueryExpr (..)
  -- , HandlerBodyLine (..)
  -- , FoxtalkExpr (..)
  )

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