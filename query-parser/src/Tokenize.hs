{-# LANGUAGE OverloadedStrings #-}

module Tokenize (
    QueryLiteral (..)
  , QueryToken(..)
  , unparse
  , handlerBody -- exported only for testing...
  , queryTokens
) where


import Data.Void
import Data.Functor

import Control.Applicative ((<|>))

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, between, manyTill, anySingle, takeRest, try, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import qualified Text.Megaparsec.Char.Lexer as L

-- DONE: Extract Literals
-- TODO: Forgotten: Extract Literals from TVarIntroLit as well!
-- TODO: Make the type of variable identifiers a type variable, parsers generate Strings
-- TODO: Add a FoxtalkType enum
-- TODO: Add types into Bindings (tokenizer can infer for TVarIntroLit)
-- TODO: Replace Void errors with String
-- TODO: Split out Query vs. Claim parsing, have errors if `/x/` appears in claims.
type Parser = Parsec Void String

data QueryLiteral =
  LSymbol String
  deriving (Show, Eq, Ord)

data QueryToken =
      TLit QueryLiteral
    | TVarIntro String
    | TVarBinding String
    | TVarIntroLit String QueryLiteral
    | THandlerBody String
    | TLParen | TRParen
    | TOr
    | TAnd
    | TWhen
    | TForAll
    | TClaim
    deriving (Show, Eq, Ord)

unparseLit :: QueryLiteral -> String
unparseLit (LSymbol s) = s

unparse :: QueryToken -> String
unparse (TLit l)           = unparseLit l
unparse (TVarIntro s)      = "/" ++ s ++ "/"
unparse (TVarIntroLit b l) = "/" ++ b ++ "/@" ++ unparseLit l
unparse (TVarBinding s)    = "(" ++ s ++ ")"
unparse TLParen            = "("
unparse TRParen            = ")"
unparse TOr                = "or"
unparse TAnd               = "and"
unparse TWhen              = "When"
unparse TForAll            = "ForAll"
unparse TClaim             = "Claim"

whenWord :: Parser QueryToken
whenWord = string "When" $> TWhen

claimWord :: Parser QueryToken
claimWord = string "Claim" $> TClaim

forAllWord :: Parser QueryToken
forAllWord = string "ForAll" $> TForAll

orWord :: Parser QueryToken
orWord = string "or" $> TOr

andWord :: Parser QueryToken
andWord = string "and" $> TAnd

lParen :: Parser QueryToken
lParen = char '(' $> TLParen

rParen :: Parser QueryToken
rParen = char ')' $> TRParen

-- TODO Fox: This could be far more readable.
handlerBody :: Parser String
handlerBody = s *> (concat <$> manyTill p e)
  where
    p :: Parser String
    p = ((\bod -> "{" ++ bod ++ "}") <$> handlerBody) <|> ((:[]) <$> anySingle)
    s :: Parser String
    s = string "{"
    e :: Parser String
    e = string "}"

ident :: Parser String
ident = some letterChar

lit :: Parser QueryLiteral
lit = choice [
    LSymbol <$> ident
  ]

varBinding :: Parser QueryToken
varBinding = do
  _ <- char '('
  s <- ident
  _ <- char ')'

  return $ TVarBinding s

varIntro :: Parser QueryToken
varIntro = do
    _ <- char '/'
    s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> lit

    case boundLit of
        Nothing -> return $ TVarIntro s
        Just lit -> return $ TVarIntroLit s lit
        _ -> undefined

queryToken :: Parser QueryToken
queryToken = choice [
      whenWord, forAllWord,
      claimWord,
      andWord, orWord,
      try varBinding,
      lParen, rParen,
      THandlerBody <$> handlerBody,
      varIntro,
      TLit <$> lit
    ]

queryTokensParser :: Parser [QueryToken]
queryTokensParser = many (queryToken <* skipMany spaceChar)

queryTokens :: String -> Maybe [QueryToken]
queryTokens = parseMaybe queryTokensParser