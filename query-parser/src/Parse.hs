{-# LANGUAGE OverloadedStrings, FlexibleInstances #-}

module Parse (
    QueryLiteral (..)
  , QueryValue (..)
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

import Tokenize (QueryToken (..), QueryLiteral(..), queryTokens)

type Parser = Parsec Void [QueryToken String]

instance (Show a, Ord a) => VisualStream [a] where
  showTokens Proxy = show

instance (Ord a) => TraversableStream [a] where
  reachOffsetNoLine 0 p = p
  reachOffsetNoLine offset p = undefined

data QueryValue a =
    VLit QueryLiteral
  | VVarIntro a
  | VVarBinding a
  | VVarIntroLit a QueryLiteral
  deriving (Show, Eq)

data QueryExpr a =
    EQueryTuple [QueryValue a]
  | EQueryAnd (QueryExpr a) (QueryExpr a)
  | EQueryOr (QueryExpr a) (QueryExpr a)
  deriving (Show, Eq)

data HandlerBodyLine a =
    BCodeLine String
  | BFoxtalkExpr (FoxtalkExpr a)
  deriving (Show, Eq)

data FoxtalkExpr a =
    EWhen (QueryExpr a) [HandlerBodyLine a]
  | EForAll String (QueryExpr a) [HandlerBodyLine a]
  | EClaim [(QueryValue a)]
  deriving (Show, Eq)


parseLit :: Parser QueryLiteral
parseLit = token sym Set.empty
  where sym (TLit l) = Just l
        sym _        = Nothing

varIntro :: Parser String
varIntro = token sym Set.empty
  where sym (TVarIntro s) = Just s
        sym _             = Nothing

varBinding :: Parser String
varBinding = token sym Set.empty
  where sym (TVarBinding s) = Just s
        sym _               = Nothing

varIntroLit :: Parser (String, QueryLiteral)
varIntroLit = token sym Set.empty
  where sym (TVarIntroLit s b) = Just (s, b)
        sym _                  = Nothing

handlerBody :: Parser [HandlerBodyLine String]
handlerBody = bod <$> token sym Set.empty
  where
    sym (THandlerBody bod) = Just bod
    sym _                  = Nothing

    bod :: String -> [HandlerBodyLine String]
    bod src = map bodLine $ lines src

    bodLine :: String -> HandlerBodyLine String
    bodLine ln =
      case queryTokens ln >>= parseMaybe foxtalkExpr of
        Just expr -> BFoxtalkExpr expr
        Nothing   -> BCodeLine ln

queryValue :: Parser (QueryValue String)
queryValue = choice [
      VLit <$> parseLit
    , VVarIntro <$> varIntro
    , VVarBinding <$> varBinding
    , (uncurry VVarIntroLit) <$> varIntroLit
    ]

queryOper :: Parser (QueryToken String)
queryOper = single TAnd <|> single TOr

queryBranch :: Parser (QueryToken String, QueryExpr String)
queryBranch = do
  oper <- queryOper
  rest <- query
  return $ (oper, rest)

query :: Parser (QueryExpr String)
query = do
  vals      <- some queryValue
  maybeRest <- optional $ queryBranch
  case maybeRest of
    Nothing           -> return $ EQueryTuple vals
    Just (TAnd, rest) -> return $ EQueryAnd (EQueryTuple vals) rest
    Just (TOr, rest)  -> return $ EQueryOr (EQueryTuple vals) rest

-- When /x/ is a husky
foxtalkWhen :: Parser (FoxtalkExpr String)
foxtalkWhen =
  do
    single TWhen
    q   <- query
    bod <- handlerBody
    return $ EWhen q bod

-- Forall /huskies/ When /x/ is a husky
foxtalkForall :: Parser (FoxtalkExpr String)
foxtalkForall =
  do
    single TForAll
    resultsVar  <- varIntro
    EWhen q bod <- foxtalkWhen
    return $ EForAll resultsVar q bod

foxtalkClaim :: Parser (FoxtalkExpr String)
foxtalkClaim =
  do
    single TClaim
    vals <- many queryValue
    return $ EClaim vals

foxtalkExpr :: Parser (FoxtalkExpr String)
foxtalkExpr = choice [ foxtalkClaim, foxtalkForall, foxtalkWhen ]

foxtalkProgram :: Parser [FoxtalkExpr String]
foxtalkProgram = some foxtalkExpr

parseProgram :: String -> Maybe [FoxtalkExpr String]
parseProgram src = queryTokens src >>= parseMaybe foxtalkProgram