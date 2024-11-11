import Test.Tasty
import Test.Tasty.HUnit
import Test.Tasty.QuickCheck as QC

import Data.List (sort)

import Token (queryTokens, QueryToken(..))
import Foxtalk (QueryValue(..), QueryExpr(..), FoxtalkExpr(..), query)

import Text.Megaparsec (parseMaybe)

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Tests" [queryTokenTests, parserTests {-, qcProps -}]

-- qcProps :: TestTree
-- qcProps = testGroup "(checked by quickcheck)"
--   [ ]


queryTokenTests :: TestTree
queryTokenTests = testGroup "Token tests"
  [
      testCase "Ident 1" $ queryTokens "a" @?= Just [TSymbolLit "a"]

    , testCase "Ident 1" $ queryTokens "fox" @?= Just [TSymbolLit "fox"]

    , testCase "Ident 3" $ queryTokens "foxorlexi" @?= Just [TSymbolLit "foxorlexi"]

    , testCase "VarIntro 1" $ queryTokens "/lexi/" @?= Just [TVarIntro "lexi"]

    , testCase "VarIntro 2" $ queryTokens "/test with spaces/" @?= Nothing

    , testCase "VarBinding 1" $ queryTokens "(lexi)" @?= Just [TVarBinding "lexi"]

    , testCase "VarBinding 2" $ queryTokens "(test with spaces)" @?= Just [TLParen, TSymbolLit "test", TSymbolLit "with", TSymbolLit "spaces", TRParen]

    , testCase "BoundLit 1" $ queryTokens "/shape/@circle" @?= Just [TVarIntroLit "shape" "circle"]

    , testCase "Tokens 1" $ queryTokens "/shape/@rectangle with x /x/ or /shape/@circle with r /r/"
        @?= Just [TVarIntroLit "shape" "rectangle", TSymbolLit "with", TSymbolLit "x", TVarIntro "x", TOr, TVarIntroLit "shape" "circle", TSymbolLit "with", TSymbolLit "r", TVarIntro "r"]

    , testCase "Tokens 2" $ queryTokens "((you) is a rectangle with x /x/) and ((you) has color /c/)"
        @?= Just ([TLParen, TVarBinding "you"]
              ++ map TSymbolLit (words "is a rectangle with x")
              ++ [TVarIntro "x", TRParen, TAnd, TLParen, TVarBinding "you"]
              ++ map TSymbolLit (words "has color")
              ++ [TVarIntro "c", TRParen])
  ]


parserTests :: TestTree
parserTests = testGroup "Parser Tests"
  [
      testCase "Parses Tuple" $ (queryTokens "/who/ is a husky" >>= parseMaybe query)
        @?= (Just (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]))

    , testCase "Parses And" $ (queryTokens "/who/ is a husky and (who) is cool" >>= parseMaybe query)
        @?= (Just (EQueryAnd (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]) (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"])))

    , testCase "Parses Or" $ (queryTokens "/who/ is a husky or (who) is cool" >>= parseMaybe query)
        @?= (Just (EQueryOr (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]) (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"])))
  ]