{-# LANGUAGE OverloadedStrings #-}

module Token(
  QueryToken(..)
  , unparse
  , tokens
) where


import Data.Void
import Data.Functor

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, choice, skipMany, many, some, optional)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

type Parser = Parsec Void String

data QueryToken =
      Ident String
    | Binding String
    | BoundLit String String
    | LParen
    | RParen
    | Or
    | And

    deriving Show

unparse :: QueryToken -> String
unparse (Ident s)      = s
unparse (Binding s)    = "/" ++ s ++ "/"
unparse (BoundLit b l) = "/" ++ b ++ "/@" ++ l
unparse LParen         = "("
unparse RParen         = ")"
unparse Or             = "or"
unparse And            = "and"

orWord :: Parser QueryToken
orWord = string "or" $> Or

andWord :: Parser QueryToken
andWord = string "and" $> And

lParen :: Parser QueryToken
lParen = char '(' $> LParen

rParen :: Parser QueryToken
rParen = char ')' $> RParen

ident :: Parser QueryToken
ident = Ident <$> some letterChar

binding :: Parser QueryToken
binding = do
    _ <- char '/'
    Ident s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> ident

    case boundLit of
        Nothing -> return $ Binding s
        Just (Ident lit) -> return $ BoundLit s lit

token :: Parser QueryToken
token = choice [
        andWord, orWord, lParen, rParen,
        binding, ident
    ]

tokens :: Parser [QueryToken]
tokens = many (token <* skipMany spaceChar)
