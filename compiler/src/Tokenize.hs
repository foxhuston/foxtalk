{-# LANGUAGE OverloadedStrings #-}

module Tokenize (
    FoxtalkType(..)
  , QueryLiteral(..)
  , QueryToken(..)
  , FoxtalkParseError(..)
  -- , unparse
  , handlerBody -- exported only for testing...
  , queryTokensParser
  , queryTokens
) where


import Data.Functor
import Data.Int (Int64)
import Data.Word (Word64)

import Data.Scientific (floatingOrInteger)

import Control.Applicative ((<|>))

-- import Data.Attoparsec.Text
import Text.Megaparsec (
    Parsec
  , manyTill
  , anySingle
  , try
  , choice
  , skipMany
  , many
  , some
  , optional
  , errorBundlePretty
  , runParser
  , ShowErrorComponent(..)
  )

import Text.Megaparsec.Char (string, char, letterChar, spaceChar)

import qualified Text.Megaparsec.Char.Lexer as L

-- DONE: Extract Literals
-- DONE: Forgotten: Extract Literals from TVarIntroLit as well!
-- DONE: Make the type of variable identifiers a type variable, parsers generate Strings
-- DONE: Write some tests that transform the type, just to make sure I've got all the cases
-- DONE: Add the rest of the literals
-- Done: Add a FoxtalkType enum
-- Done: Add types into Bindings (tokenizer can infer for TVarIntroLit)
-- TODO: (SORT OF DONE) Replace Void errors with String
-- TODO: #include & pkg-config
-- TODO: Locals
-- TODO: Poll, FreeTuple
-- TODO: Split out Query vs. Claim parsing, have errors if `/x/` appears in claims.
-- TODO: Conjunction emits a cross-product in handler; Disjunction does ???


newtype FoxtalkParseError = FoxtalkParseError(String)
  deriving (Eq, Ord)

instance ShowErrorComponent FoxtalkParseError where
  showErrorComponent (FoxtalkParseError msg) = msg

type Parser = Parsec FoxtalkParseError String

data FoxtalkType = TSymbol | TCptr | TU64 | TI64 | TDouble | TBytes | TRest
  deriving (Show, Eq, Ord)

data QueryLiteral =
  LSymbol String
  | LU64 Word64
  | LI64 Int64
  | LDouble Double
  deriving (Show, Eq, Ord)

data QueryToken a =
      TLit QueryLiteral
    | TUntypedVarIntro a
    | TVarIntro FoxtalkType a
    | TVarBinding a
    | TVarIntroLit FoxtalkType a QueryLiteral
    | THandlerBody String
    | TLParen | TRParen
    | TOr
    | TAnd
    | TWhen
    | TForAll
    | TClaim
    | TLocals
    | TPoll
    | TFreeTuple
    | TPrefix
    deriving (Show, Eq, Ord, Functor, Foldable, Traversable)

----- UTILITIES -----

skipSpaces :: Parser ()
skipSpaces = skipMany spaceChar

lexeme = L.lexeme skipSpaces
symbol = L.symbol skipSpaces

----- PARSING TYPES -----

parseType :: Parser FoxtalkType
parseType = choice [
    string "symbol" $> TSymbol
  , string "ptr"    $> TCptr
  , string "u64"    $> TU64
  , string "i64"    $> TI64
  , string "double" $> TDouble
  , string "bytes"  $> TBytes
  , string "..."    $> TRest
  ]

typeOfLit :: QueryLiteral -> FoxtalkType
typeOfLit (LSymbol _)  = TSymbol
typeOfLit (LU64 _)     = TU64
typeOfLit (LI64 _)     = TI64
typeOfLit (LDouble _)  = TDouble

----- PARSING KEYWORDS -----

keyword :: Parser (QueryToken a)
keyword = choice $ map (\(s, t) -> string s $> t) kws
  where
    kws :: [(String, QueryToken a)]
    kws = [
        ("When", TWhen)
      , ("Claim", TClaim)
      , ("ForAll", TForAll)
      , ("or", TOr)
      , ("and", TAnd)
      , ("Locals", TLocals)
      , ("Poll", TPoll)
      , ("FreeTuple", TFreeTuple)
      , ("...", TPrefix)
      ]

lParen :: Parser (QueryToken a)
lParen = char '(' $> TLParen

rParen :: Parser (QueryToken a)
rParen = char ')' $> TRParen

----- PARSING TOKENS -----

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
ident = some (letterChar <|> char '_')

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
    ty <- optional $ symbol ":"

    case ty of
      Nothing ->
        do
          _ <- char '/'
          return $ TUntypedVarIntro s
      Just _ ->
        do
          t <- lexeme parseType
          _ <- char '/'
          return $ TVarIntro t s

varIntroLit :: Parser (QueryToken String)
varIntroLit = do
    _ <- char '/'
    s <- ident
    _ <- char '/'
    boundLit <- char '@' *> lit
    return $ TVarIntroLit (typeOfLit boundLit) s boundLit

queryToken :: Parser (QueryToken String)
queryToken = choice [
      keyword,
      try varBinding,
      lParen, rParen,
      THandlerBody <$> handlerBody,
      try varIntroLit,
      varIntro,
      TLit <$> lit
    ]

queryTokensParser :: Parser [QueryToken String]
queryTokensParser = skipMany spaceChar *> many (queryToken <* skipMany spaceChar)

queryTokens :: String -> String -> Either String [QueryToken String]
queryTokens fileName source =
  case runParser queryTokensParser fileName source of
    Left err -> Left (errorBundlePretty err)
    Right o -> Right o