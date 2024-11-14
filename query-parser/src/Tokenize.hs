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
import Data.Int (Int64)
import Data.Word (Word64)

import Data.Scientific (floatingOrInteger)

import Control.Applicative ((<|>))

-- import Data.Attoparsec.Text
import Text.Megaparsec (Parsec, between, manyTill, anySingle, takeRest, try, choice, skipMany, many, some, optional, parseMaybe)
import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import qualified Text.Megaparsec.Char.Lexer as L

-- DONE: Extract Literals
-- DONE: Forgotten: Extract Literals from TVarIntroLit as well!
-- DONE: Make the type of variable identifiers a type variable, parsers generate Strings
-- DONE: Write some tests that transform the type, just to make sure I've got all the cases
-- TODO: Add the rest of the literals
-- TODO: Add a FoxtalkType enum
-- TODO: Add types into Bindings (tokenizer can infer for TVarIntroLit)
-- TODO: Replace Void errors with String
-- TODO: Split out Query vs. Claim parsing, have errors if `/x/` appears in claims.
type Parser = Parsec Void String

data FoxtalkType =
  TQuery | TPrefix | TSymbol | TCptr | TU64 | TI64
  | TDouble | TBytes

data QueryLiteral =
  LSymbol String
  | LU64 Word64
  | LI64 Int64
  | LDouble Double
  deriving (Show, Eq, Ord)

data QueryToken a =
      TLit QueryLiteral
    | TVarIntro a
    | TVarBinding a
    | TVarIntroLit a QueryLiteral
    | THandlerBody String
    | TLParen | TRParen
    | TOr
    | TAnd
    | TWhen
    | TForAll
    | TClaim
    deriving (Show, Eq, Ord, Functor, Foldable, Traversable)

unparseLit :: QueryLiteral -> String
unparseLit (LSymbol s)  = s
unparseLit (LU64 u)     = show u
unparseLit (LI64 i)     = show i
unparseLit (LDouble d)  = show d

unparse :: Show a => QueryToken a -> String
unparse (TLit l)           = unparseLit l
unparse (TVarIntro s)      = "/" ++ show s ++ "/"
unparse (TVarIntroLit b l) = "/" ++ show b ++ "/@" ++ unparseLit l
unparse (TVarBinding s)    = "(" ++ show s ++ ")"
unparse TLParen            = "("
unparse TRParen            = ")"
unparse TOr                = "or"
unparse TAnd               = "and"
unparse TWhen              = "When"
unparse TForAll            = "ForAll"
unparse TClaim             = "Claim"

whenWord :: Parser (QueryToken a)
whenWord = string "When" $> TWhen

claimWord :: Parser (QueryToken a)
claimWord = string "Claim" $> TClaim

forAllWord :: Parser (QueryToken a)
forAllWord = string "ForAll" $> TForAll

orWord :: Parser (QueryToken a)
orWord = string "or" $> TOr

andWord :: Parser (QueryToken a)
andWord = string "and" $> TAnd

lParen :: Parser (QueryToken a)
lParen = char '(' $> TLParen

rParen :: Parser (QueryToken a)
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

numberLit :: Parser QueryLiteral
numberLit = do
  n <- L.signed (skipMany spaceChar) L.scientific
  c <- optional $ char 'u'
  case c of
    Nothing -> case floatingOrInteger n of
                  Left flt -> return $ LDouble flt
                  Right intg -> return $ LI64 intg
    Just _  -> case floatingOrInteger n of
                  Left flt -> undefined -- TODO: Better failure message
                  Right intg -> return $ LU64 intg

lit :: Parser QueryLiteral
lit = choice [
      numberLit
    , LSymbol <$> ident
  ]

varBinding :: Parser (QueryToken String)
varBinding = do
  _ <- char '('
  s <- ident
  _ <- char ')'

  return $ TVarBinding s

varIntro :: Parser (QueryToken String)
varIntro = do
    _ <- char '/'
    s <- ident
    _ <- char '/'

    boundLit <- optional $ char '@' *> lit

    case boundLit of
        Nothing -> return $ TVarIntro s
        Just lit -> return $ TVarIntroLit s lit
        _ -> undefined

queryToken :: Parser (QueryToken String)
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

queryTokensParser :: Parser [QueryToken String]
queryTokensParser = many (queryToken <* skipMany spaceChar)

queryTokens :: String -> Maybe [QueryToken String]
queryTokens = parseMaybe queryTokensParser