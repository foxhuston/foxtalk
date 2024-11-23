{-# LANGUAGE OverloadedStrings, FlexibleInstances, TemplateHaskell #-}

module Parse (
    QueryLiteral(..)

  , QueryValue(..)

  , QueryExpr(..)
  , QueryExprF(..)

  , HandlerBodyLine(..)
  , HandlerBodyLineF(..)

  , FoxtalkExpr(..)
  , FoxtalkExprF(..)

  , FoxtalkType(..)

  , parseProgram

  , query -- Exported only for testing
  , foxtalkProgram -- Exported only for testing

  , foxtalkWhen -- Exported only for testing
  , foxtalkForall -- Exported only for testing
  , foxtalkClaim -- Exported only for testing
) where


import Data.Void
import Data.Functor
import Data.Functor.Foldable.TH (makeBaseFunctor)

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
  , runParser, errorBundlePretty
  )

import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import Tokenize (
    FoxtalkType(..)
  , QueryToken(..)
  , QueryLiteral(..)
  , FoxtalkParseError

  , queryTokens
  )

type Parser = Parsec FoxtalkParseError [QueryToken String]

instance (Show a, Ord a) => VisualStream [a] where
  showTokens Proxy = show

instance (Ord a) => TraversableStream [a] where
  reachOffsetNoLine 0 p = p
  reachOffsetNoLine offset p = undefined

data QueryValue a =
    VLit QueryLiteral
  | VVarIntro FoxtalkType a
  | VVarBinding a
  | VVarIntroLit FoxtalkType a QueryLiteral
  deriving (Show, Eq, Functor, Foldable, Traversable)

data QueryExpr a =
    EQueryTuple [QueryValue a]
  | EQueryAnd (QueryExpr a) (QueryExpr a)
  | EQueryOr (QueryExpr a) (QueryExpr a)
  deriving (Show, Eq, Functor, Foldable, Traversable)

data HandlerBodyLine a =
    BCodeLine String
  | BFoxtalkExpr (FoxtalkExpr a)
  deriving (Show, Eq, Functor, Foldable, Traversable)

data FoxtalkExpr a =
    EWhen (QueryExpr a) [HandlerBodyLine a]
  | EForAll a (QueryExpr a) [HandlerBodyLine a]
  | EClaim [(QueryValue a)]
  deriving (Show, Eq, Functor, Foldable, Traversable)


makeBaseFunctor ''QueryExpr
makeBaseFunctor ''HandlerBodyLine
makeBaseFunctor ''FoxtalkExpr

parseLit :: Parser QueryLiteral
parseLit = token sym Set.empty
  where sym (TLit l) = Just l
        sym _        = Nothing

varIntro :: Parser (QueryValue String)
varIntro = token sym Set.empty
  where sym (TVarIntro t s) = Just $ VVarIntro t s
        sym _               = Nothing

untypedVarIntro :: Parser String
untypedVarIntro = token sym Set.empty
  where sym (TUntypedVarIntro s) = Just $ s
        sym _                    = Nothing

varBinding :: Parser (QueryValue String)
varBinding = token sym Set.empty
  where sym (TVarBinding s) = Just $ VVarBinding s
        sym _               = Nothing

varIntroLit :: Parser (QueryValue String)
varIntroLit = token sym Set.empty
  where sym (TVarIntroLit t s b) = Just $ VVarIntroLit t s b
        sym _                    = Nothing

handlerBody :: Parser [HandlerBodyLine String]
handlerBody = bod <$> token sym Set.empty
  where
    sym (THandlerBody bod) = Just bod
    sym _                  = Nothing

    bod :: String -> [HandlerBodyLine String]
    bod src = map bodLine $ lines src

    bodLine :: String -> HandlerBodyLine String
    bodLine ln =
      case queryTokens "BODLINE" ln of
        Right tokens ->
          case runParser foxtalkExpr "BODLINE" tokens of
            Right expr -> BFoxtalkExpr expr
            Left _     -> BCodeLine ln
        Left _     -> BCodeLine ln

queryValue :: Parser (QueryValue String)
queryValue = choice [
      VLit <$> parseLit
    , varIntro
    , varBinding
    , varIntroLit
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
    resultsVar  <- untypedVarIntro
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

parseProgram :: String -> String -> Either String [FoxtalkExpr String]
parseProgram fileName src =
  case queryTokens src fileName of
    Right tokens ->
      case runParser foxtalkProgram fileName tokens of
        Right exprs -> Right exprs
        Left errs   -> Left (errorBundlePretty errs)
    Left err     -> Left err