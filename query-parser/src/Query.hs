{-# LANGUAGE  TypeOperators #-}

module Query (toQuery, bindMappings) where

import Token
import Tuple

import Control.Applicative ((<|>))

import Text.Megaparsec (MonadParsec, Token, Parsec, choice, skipMany, many, some, optional, single, sepBy)

import Data.Void

data QueryExpr =
  ESymbol
  | EVarIntro

type TokenParser = Parsec Void QueryToken

inner :: TokenParser TupleNoun
inner = undefined

-- tq :: TokenParser [Tuple]
-- tq = sepBy (single LParen *> (Tuple <$> some inner) <* single RParen) (single Or <|> single And)

toQuery :: [QueryToken] -> [TupleNoun]
toQuery [] = []
toQuery ((TSymbolLit s):xs) = Symbol s : toQuery xs
toQuery ((TVarIntro _):xs) = Query : toQuery xs
toQuery ((TVarIntroLit _ lit):xs) = Symbol lit : toQuery xs
toQuery _ = undefined


bindMappings :: [QueryToken] -> Tuple -> Maybe [(String, TupleNoun)]
bindMappings = undefined