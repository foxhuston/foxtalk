{-# LANGUAGE OverloadedStrings, FlexibleInstances #-}

module Foxtalk (
    QueryValue (..)
  , QueryExpr (..)
  , FoxtalkExpr (..)

  , query

  , foxtalkWhen -- Exported only for testing
  , foxtalkForall -- Exported only for testing
) where


import Data.Void
import Data.Functor

import Data.Maybe (fromMaybe)
import Data.Proxy (Proxy (..))

import Control.Applicative ((<|>))

import qualified Data.Set as Set

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, VisualStream (..), TraversableStream (..), single, token, try, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import Token (QueryToken (..))

type Parser = Parsec Void [QueryToken]

instance (Show a, Ord a) => VisualStream [a] where
  showTokens Proxy = show

instance (Ord a) => TraversableStream [a] where
  reachOffsetNoLine 0 p = p
  reachOffsetNoLine offset p = undefined

data QueryValue =
    VSymbolLit String
  | VVarIntro String
  | VVarBinding String
  | VVarIntroLit String String
  deriving (Show, Eq)

data QueryExpr =
    EQueryTuple [QueryValue]
  | EQueryAnd QueryExpr QueryExpr
  | EQueryOr QueryExpr QueryExpr
  deriving (Show, Eq)

data FoxtalkExpr =
    EWhen QueryExpr String
  | EForAll String QueryExpr String
  deriving (Show, Eq)


symbolLit :: Parser String
symbolLit = token sym Set.empty
  where sym (TSymbolLit s) = Just s
        sym _              = Nothing

varIntro :: Parser String
varIntro = token sym Set.empty
  where sym (TVarIntro s) = Just s
        sym _             = Nothing


varBinding :: Parser String
varBinding = token sym Set.empty
  where sym (TVarBinding s) = Just s
        sym _               = Nothing


varIntroLit :: Parser (String, String)
varIntroLit = token sym Set.empty
  where sym (TVarIntroLit s b) = Just (s, b)
        sym _                  = Nothing

handlerBody :: Parser String
handlerBody = token sym Set.empty
  where
    sym (THandlerBody bod) = Just bod
    sym _                  = Nothing

queryValue :: Parser QueryValue
queryValue = choice [
      VSymbolLit <$> symbolLit
    , VVarIntro <$> varIntro
    , VVarBinding <$> varBinding
    , (uncurry VVarIntroLit) <$> varIntroLit
    ]

queryOper :: Parser QueryToken
queryOper = single TAnd <|> single TOr

queryBranch :: Parser (QueryToken, QueryExpr)
queryBranch = do
  oper <- queryOper
  rest <- query
  return $ (oper, rest)

query :: Parser QueryExpr
query = do
  vals <- some queryValue
  maybeRest <- optional $ queryBranch
  case maybeRest of
    Nothing -> return $ EQueryTuple vals
    Just (TAnd, rest) -> return $ EQueryAnd (EQueryTuple vals) rest
    Just (TOr, rest) -> return $ EQueryOr (EQueryTuple vals) rest


-- When /x/ is a husky
foxtalkWhen :: Parser FoxtalkExpr
foxtalkWhen =
  do
    single TWhen
    q <- query
    bod <- handlerBody
    return $ EWhen q bod

-- Forall /huskies/ When /x/ is a husky
foxtalkForall :: Parser FoxtalkExpr
foxtalkForall =
  do
    single TForAll
    resultsVar <- varIntro
    EWhen q bod <- foxtalkWhen
    return $ EForAll resultsVar q bod

foxtalkProgram :: Parser FoxtalkExpr
foxtalkProgram = undefined