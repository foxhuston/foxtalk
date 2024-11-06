{-# LANGUAGE OverloadedStrings #-}

module Token(
  Token(..)
  , unparse
  , tokens
) where


import Data.Void
import Data.Functor

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, choice, skipMany, many, some, optional)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

type Parser = Parsec Void String

data Token =
      Ident String
    | Binding String
    | BoundLit String String
    | LParen
    | RParen
    | Or
    | And

    deriving Show

unparse :: Token -> String
unparse (Ident s)      = s
unparse (Binding s)    = "/" ++ s ++ "/"
unparse (BoundLit b l) = "/" ++ b ++ "/@" ++ l
unparse LParen         = "("
unparse RParen         = ")"
unparse Or             = "or"
unparse And            = "and"

orWord :: Parser Token
orWord = string "or" $> Or

andWord :: Parser Token
andWord = string "and" $> And

lParen :: Parser Token
lParen = char '(' $> LParen

rParen :: Parser Token
rParen = char ')' $> RParen

ident :: Parser Token
ident = Ident <$> some letterChar

binding :: Parser Token
binding = do
    _ <- char '/'
    Ident s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> ident

    case boundLit of
        Nothing -> return $ Binding s
        Just (Ident lit) -> return $ BoundLit s lit

token :: Parser Token
token = choice [
        andWord, orWord, lParen, rParen,
        binding, ident
    ]

tokens :: Parser [Token]
tokens = many (token <* skipMany spaceChar)
