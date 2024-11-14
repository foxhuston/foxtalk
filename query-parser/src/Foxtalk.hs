module Foxtalk (
  queryValueToTupleEntry -- for testing only
  , tupleObj -- for testing only
) where

import Data.List (intercalate)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)
  )

queryValueToTupleEntry :: QueryValue -> String
queryValueToTupleEntry (VLit (LSymbol s)) = "{\"" ++ s ++ "\"}"
queryValueToTupleEntry (VLit _)           = undefined
queryValueToTupleEntry (VVarIntro s)      = "TupleValue::query()"
queryValueToTupleEntry (VVarBinding s)    = s
queryValueToTupleEntry (VVarIntroLit s _) = "TupleValue::query()"

tupleObj :: String -> [QueryValue] -> String
tupleObj name values =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in "Tuple " ++ name ++ " { " ++ vs ++ "}"