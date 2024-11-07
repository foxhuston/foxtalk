{-# LANGUAGE  TypeOperators #-}

module Query (toQuery, bindMappings) where

import Token
import Tuple

import Control.Applicative ((<|>))

import Text.Megaparsec (MonadParsec, Token, Parsec, choice, skipMany, many, some, optional, single, sepBy)

import Data.Void

type TokenParser = Parsec Void QueryToken

inner :: TokenParser TupleNoun
inner = undefined

tq :: TokenParser [Tuple]
tq = sepBy (single LParen *> (Tuple <$> some inner) <* single RParen) (single Or <|> single And)



toQuery :: [QueryToken] -> [TupleNoun]
toQuery [] = []
toQuery ((Ident s):xs) = Symbol s : toQuery xs
toQuery ((Binding _):xs) = Query : toQuery xs
toQuery ((BoundLit _ lit):xs) = Symbol lit : toQuery xs
toQuery _ = undefined



bindMappings :: [QueryToken] -> Tuple -> Maybe [(String, TupleNoun)]
bindMappings = undefined