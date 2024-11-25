module Codegen.Disambiguate (disambiguateTuples) where

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

import Data.List (find, nub)
import Data.Functor.Foldable (cata)
import Data.Maybe (mapMaybe)

getQueryTupleValues :: forall a. QueryExpr a -> [[QueryValue a]]
getQueryTupleValues = cata go
  where
    go :: QueryExprF a [[QueryValue a]] -> [[QueryValue a]]
    go (EQueryTupleF exprs) = [exprs]
    go (EQueryAndF l r)     = l ++ r
    go (EQueryOrF l r)      = l ++ r

getTupleValues :: FoxtalkExpr a -> [[(Int, QueryValue a)]]
getTupleValues (EWhen q _)     = map (zip [0..]) $ getQueryTupleValues q
getTupleValues (EForAll _ q _) = map (zip [0..]) $ getQueryTupleValues q
getTupleValues (EClaim vs)     = [zip [0..] vs]

disambiguateTuples :: forall a. Eq a => FoxtalkExpr a -> Maybe [(Int, QueryValue a)]
disambiguateTuples expr = select tuples <$> find unambig [0..maxL]
  where
    tuples :: [[(Int, QueryValue a)]]
    tuples = getTupleValues expr

    trunc :: [[(Int, QueryValue a)]]
    trunc = map (take maxL) tuples

    select :: [[(Int, QueryValue a)]] -> Int -> [(Int, QueryValue a)]
    select ls idx = map (!!idx) ls

    maxL :: Int
    maxL = maximum $ map length tuples

    unambig :: Int -> Bool
    unambig idx = nub xs == xs
      where xs = select tuples idx
