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
import Text.Megaparsec (Parsec, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

type Parser = Parsec Void String

data QueryToken =
      TIdent String
    | TBinding String
    | TBoundLit String String
    | TLParen
    | TRParen
    | TOr
    | TAnd

    deriving (Show, Eq)

unparse :: QueryToken -> String
unparse (TIdent s)      = s
unparse (TBinding s)    = "/" ++ s ++ "/"
unparse (TBoundLit b l) = "/" ++ b ++ "/@" ++ l
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
ident = TIdent <$> some letterChar

binding :: Parser QueryToken
binding = do
    _ <- char '/'
    TIdent s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> ident

    case boundLit of
        Nothing -> return $ TBinding s
        Just (TIdent lit) -> return $ TBoundLit s lit

token :: Parser QueryToken
token = choice [
        andWord, orWord, lParen, rParen,
        binding, ident
    ]

tokensParser :: Parser [QueryToken]
tokensParser = many (token <* skipMany spaceChar)

tokens :: String -> Maybe [QueryToken]
tokens = parseMaybe tokensParser