{-# LANGUAGE OverloadedStrings, FlexibleInstances #-}

module Parse (
    QueryValue (..)
  , QueryExpr (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)

  , parseProgram

  , query -- Exported only for testing
  , foxtalkProgram -- Exported only for testing

  , foxtalkWhen -- Exported only for testing
  , foxtalkForall -- Exported only for testing
  , foxtalkClaim -- Exported only for testing
) where


import Data.Void
import Data.Functor

import Data.Maybe (fromMaybe)
import Data.Proxy (Proxy (..))

import Control.Applicative ((<|>))

import qualified Data.Set as Set

-- import Data.Attoparsec.Text
import Text.Megaparsec (
  Parsec
  , VisualStream (..)
  , TraversableStream (..)
  , single
  , token
  , try
  , choice
  , skipMany
  , many
  , some
  , optional
  , parseMaybe
  )

import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import Tokenize (QueryToken (..), queryTokens)

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

data HandlerBodyLine =
    BCodeLine String
  | BFoxtalkExpr FoxtalkExpr
  deriving (Show, Eq)

data FoxtalkExpr =
    EWhen QueryExpr [HandlerBodyLine]
  | EForAll String QueryExpr [HandlerBodyLine]
  | EClaim [QueryValue]
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

handlerBody :: Parser [HandlerBodyLine]
handlerBody = bod <$> token sym Set.empty
  where
    sym (THandlerBody bod) = Just bod
    sym _                  = Nothing

    bod :: String -> [HandlerBodyLine]
    bod src = map bodLine $ lines src

    bodLine :: String -> HandlerBodyLine
    bodLine ln =
      case queryTokens ln >>= parseMaybe foxtalkExpr of
        Just expr -> BFoxtalkExpr expr
        Nothing -> BCodeLine ln

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

foxtalkClaim :: Parser FoxtalkExpr
foxtalkClaim =
  do
    single TClaim
    vals <- many queryValue
    return $ EClaim vals

foxtalkExpr :: Parser FoxtalkExpr
foxtalkExpr = choice [ foxtalkClaim, foxtalkForall, foxtalkWhen ]

foxtalkProgram :: Parser [FoxtalkExpr]
foxtalkProgram = some foxtalkExpr

parseProgram :: String -> Maybe [FoxtalkExpr]
parseProgram src = queryTokens src >>= parseMaybe foxtalkProgram