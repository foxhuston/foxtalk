{-# LANGUAGE OverloadedStrings #-}

module Token(
  QueryToken(..)
  , unparse
  , tokens
) where


import Data.Void
import Data.Functor

import Data.Maybe (fromMaybe)

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, try, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

type Parser = Parsec Void String

data QueryToken =
      TSymbolLit String
    | TVarIntro String
    | TVarBinding String
    | TVarIntroLit String String
    | TLParen
    | TRParen
    | TOr
    | TAnd

    deriving (Show, Eq)

unparse :: QueryToken -> String
unparse (TSymbolLit s)      = s
unparse (TVarIntro s)    = "/" ++ s ++ "/"
unparse (TVarIntroLit b l) = "/" ++ b ++ "/@" ++ l
unparse (TVarBinding s) = "(" ++ s ++ ")"
unparse TLParen         = "("
unparse TRParen         = ")"
unparse TOr             = "or"
unparse TAnd            = "and"

orWord :: Parser QueryToken
orWord = string "or" $> TOr

andWord :: Parser QueryToken
andWord = string "and" $> TAnd

lParen :: Parser QueryToken
lParen = char '(' $> TLParen

rParen :: Parser QueryToken
rParen = char ')' $> TRParen

ident :: Parser QueryToken
ident = TSymbolLit <$> some letterChar

varBinding :: Parser QueryToken
varBinding = do
  _ <- char '('
  TSymbolLit s <- ident
  _ <- char ')'

  return $ TVarBinding s

varIntro :: Parser QueryToken
varIntro = do
    _ <- char '/'
    TSymbolLit s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> ident

    case boundLit of
        Nothing -> return $ TVarIntro s
        Just (TSymbolLit lit) -> return $ TVarIntroLit s lit

token :: Parser QueryToken
token = choice [
        andWord, orWord,
        try varBinding,
        lParen, rParen,
        varIntro, ident
    ]

tokensParser :: Parser [QueryToken]
tokensParser = many (token <* skipMany spaceChar)

tokens :: String -> Maybe [QueryToken]
tokens = parseMaybe tokensParser