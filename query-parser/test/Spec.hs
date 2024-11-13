import Test.Tasty
import Test.Tasty.HUnit
import Test.Tasty.QuickCheck as QC

import Data.List (sort)

import Token (queryTokens, QueryToken(..), handlerBody)
import Foxtalk (QueryValue(..), QueryExpr(..), FoxtalkExpr(..), query, foxtalkProgram, foxtalkWhen, foxtalkForall, foxtalkClaim)

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

    , testCase "When" $ queryTokens "When bonk"
        @?= Just ([TWhen, TSymbolLit "bonk"])

    , testCase "Handler Body 0" $ parseMaybe handlerBody "{ this is some c code or whatever }"
                                                @?= Just (" this is some c code or whatever ")

    , testCase "Handler Body 1" $ parseMaybe handlerBody "{ this is {some c} code or whatever }"
                                                @?= Just (" this is {some c} code or whatever ")

    , testCase "Handler Body 2" $ queryTokens  "When bonk { this is some c code or whatever }"
        @?= Just ([TWhen, TSymbolLit "bonk", THandlerBody " this is some c code or whatever "])

    , testCase "Handler Body 3" $ queryTokens  "When bonk { this is some for(;;) { gnarly} c code or whatever}"
        @?= Just ([TWhen, TSymbolLit "bonk", THandlerBody " this is some for(;;) { gnarly} c code or whatever"])

    , testCase "Tokenizes Forall Clause with Nested Bod" $
        (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }")
            @?= (Just [TForAll, TVarIntro "huskies", TWhen, TVarIntro "who"
                      , TSymbolLit "is", TSymbolLit "a", TSymbolLit "husky"
                      , THandlerBody " some {gnarlier} C code "])

    , testCase "Tokenizes top-level Claim" $
        (queryTokens "Claim lexi is a husky")
            @?= (Just [TClaim, TSymbolLit "lexi", TSymbolLit "is", TSymbolLit "a", TSymbolLit "husky"])

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

    , testCase "Parses When Clause" $ (queryTokens "When /who/ is a husky or (who) is cool { some gnarly C code }" >>= parseMaybe foxtalkWhen)
        @?= (Just (EWhen
                    (EQueryOr (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]) (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"]))
                    " some gnarly C code "))

    , testCase "Parses When Clause with Nested Bod" $
        (queryTokens "When /who/ is a husky or (who) is cool { some {gnarlier} C code }"
            >>= parseMaybe foxtalkWhen)
                @?= (Just (EWhen
                            (EQueryOr
                                (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"])
                                (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"]))
                            " some {gnarlier} C code "))

    , testCase "Parses Forall Clause with Nested Bod" $
        (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }"
            >>= parseMaybe foxtalkForall)
                @?= (Just (EForAll "huskies"
                            (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"])
                            " some {gnarlier} C code "))

    , testGroup "Top-level Parser" [
          testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EClaim [VSymbolLit "lexi",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]])

        , testCase "Parses When Clause" $
            (queryTokens "When /who/ is a husky or (who) is cool { some gnarly C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EWhen
                                (EQueryOr (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]) (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"]))
                                " some gnarly C code "])

        , testCase "Parses When Clause with Nested Bod" $
            (queryTokens "When /who/ is a husky or (who) is cool { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EWhen
                                (EQueryOr
                                    (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"])
                                    (EQueryTuple [VVarBinding "who",VSymbolLit "is",VSymbolLit "cool"]))
                                " some {gnarlier} C code "])

        , testCase "Parses Forall Clause with Nested Bod" $
            (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EForAll "huskies"
                                (EQueryTuple [VVarIntro "who",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"])
                                " some {gnarlier} C code "])

        , testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EClaim [VSymbolLit "lexi",VSymbolLit "is",VSymbolLit "a",VSymbolLit "husky"]])
    ]
  ]