{-# LANGUAGE OverloadedStrings, FlexibleInstances #-}

module Foxtalk() where


import Data.Void
import Data.Functor

import Data.Maybe (fromMaybe)
import Data.Proxy (Proxy (..))

import Control.Applicative ((<|>))

import qualified Data.Set as Set

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, VisualStream (..), single, token, try, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import Token

type Parser = Parsec Void [QueryToken]

-- instance VisualStream [QueryToken] where
--   showTokens Proxy = show

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

data FoxTalkExpr =
    EWhen QueryExpr String
  | EForAll String QueryExpr String
  deriving (Show, Eq)


-- orWord :: Parser QueryToken
-- orWord = string "or" $> TOr

-- andWord :: Parser QueryToken
-- andWord = string "and" $> TAnd

-- lParen :: Parser QueryToken
-- lParen = char '(' $> TLParen

-- rParen :: Parser QueryToken
-- rParen = char ')' $> TRParen

-- symbolLit :: Parser String
-- symbolLit = some letterChar

-- varBinding :: Parser QueryValue
-- varBinding = do
--   _ <- char '('
--   s <- symbolLit
--   _ <- char ')'

--   return $ VVarBinding s

-- varIntro :: Parser QueryValue
-- varIntro = do
--     _ <- char '/'
--     s <- symbolLit
--     _ <- char '/'

--     boundLit <- optional $ char '@' *> symbolLit

--     case boundLit of
--         Nothing -> return $ VVarIntro s
--         Just lit -> return $ VVarIntroLit s lit

-- queryValue :: Parser QueryValue
-- queryValue = choice [
--         try varBinding,
--         varIntro,
--         VSymbolLit <$> symbolLit
--     ]

symbolLit :: Parser QueryValue
symbolLit = VSymbolLit <$> token sym Set.empty
  where sym (TSymbolLit s) = Just s
        sym _              = Nothing

varIntro :: Parser QueryValue
varIntro = VVarIntro <$> token sym Set.empty
  where sym (TVarIntro s) = Just s
        sym _             = Nothing


varBinding :: Parser QueryValue
varBinding = VVarBinding <$> token sym Set.empty
  where sym (TVarBinding s) = Just s
        sym _               = Nothing


varIntroLit :: Parser QueryValue
varIntroLit = (uncurry VVarIntroLit) <$> token sym Set.empty
  where sym (TVarIntroLit s b) = Just (s, b)
        sym _                  = Nothing


queryValue :: Parser QueryValue
queryValue = choice [symbolLit, varIntro, varBinding, varIntroLit]

queryOper :: Parser QueryToken
queryOper = single TAnd <|> single TOr

queryBranch :: Parser (QueryToken, QueryExpr)
queryBranch = do
  oper <- queryOper
  rest <- query
  return $ (oper, rest)

queryTuple :: Parser QueryExpr
queryTuple = do
  vals <- some queryValue
  maybeRest <- optional $ queryBranch
  case maybeRest of
    Nothing -> return $ EQueryTuple vals
    Just (TAnd, rest) -> return $ EQueryAnd (EQueryTuple vals) rest
    Just (TOr, rest) -> return $ EQueryOr (EQueryTuple vals) rest

query :: Parser QueryExpr
query = choice [queryTuple]