module Parsing (parsingTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Tokenize (queryTokens, QueryLiteral (..), QueryToken(..), handlerBody)
import Parse (
      QueryValue(..)
    , QueryExpr(..)
    , HandlerBodyLine(..)
    , FoxtalkExpr(..)
    , query
    , foxtalkProgram
    , foxtalkWhen
    , foxtalkForall
    , foxtalkClaim
    )

import Text.Megaparsec (parseMaybe)

parsingTests :: TestTree
parsingTests = testGroup "Parsing Tests" [queryTokenTests, parserTests]

queryTokenTests :: TestTree
queryTokenTests = testGroup "Token tests"
  [
      testCase "Ident 1" $ queryTokens "a" @?= Just [TLit (LSymbol "a")]

    , testCase "Ident 1" $ queryTokens "fox" @?= Just [TLit (LSymbol "fox")]

    , testCase "Ident 3" $ queryTokens "foxorlexi" @?= Just [TLit (LSymbol "foxorlexi")]

    , testCase "VarIntro 1" $ queryTokens "/lexi/" @?= Just [TVarIntro "lexi"]

    , testCase "VarIntro 2" $ queryTokens "/test with spaces/" @?= Nothing

    , testCase "VarBinding 1" $ queryTokens "(lexi)" @?= Just [TVarBinding "lexi"]

    , testCase "VarBinding 2" $ queryTokens "(test with spaces)" @?= Just [TLParen, TLit (LSymbol "test"), TLit (LSymbol "with"), TLit (LSymbol "spaces"), TRParen]

    , testCase "BoundLit 1" $ queryTokens "/shape/@circle" @?= Just [TVarIntroLit "shape" "circle"]

    , testCase "Tokens 1" $ queryTokens "/shape/@rectangle with x /x/ or /shape/@circle with r /r/"
        @?= Just [TVarIntroLit "shape" "rectangle", TLit (LSymbol "with"), TLit (LSymbol "x"), TVarIntro "x", TOr, TVarIntroLit "shape" "circle", TLit (LSymbol "with"), TLit (LSymbol "r"), TVarIntro "r"]

    , testCase "Tokens 2" $ queryTokens "((you) is a rectangle with x /x/) and ((you) has color /c/)"
        @?= Just ([TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "is a rectangle with x")
              ++ [TVarIntro "x", TRParen, TAnd, TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "has color")
              ++ [TVarIntro "c", TRParen])

    , testCase "When" $ queryTokens "When bonk"
        @?= Just ([TWhen, TLit (LSymbol "bonk")])

    , testCase "Handler Body 0" $ parseMaybe handlerBody "{ this is some c code or whatever }"
                                                @?= Just (" this is some c code or whatever ")

    , testCase "Handler Body 1" $ parseMaybe handlerBody "{ this is {some c} code or whatever }"
                                                @?= Just (" this is {some c} code or whatever ")

    , testCase "Handler Body 2" $ queryTokens  "When bonk { this is some c code or whatever }"
        @?= Just ([TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some c code or whatever "])

    , testCase "Handler Body 3" $ queryTokens  "When bonk { this is some for(;;) { gnarly} c code or whatever}"
        @?= Just ([TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some for(;;) { gnarly} c code or whatever"])

    , testCase "Tokenizes Forall Clause with Nested Bod" $
        (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }")
            @?= (Just [TForAll, TVarIntro "huskies", TWhen, TVarIntro "who"
                      , TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")
                      , THandlerBody " some {gnarlier} C code "])

    , testCase "Tokenizes top-level Claim" $
        (queryTokens "Claim lexi is a husky")
            @?= (Just [TClaim, TLit (LSymbol "lexi"), TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")])

  ]


parserTests :: TestTree
parserTests = testGroup "Parser Tests"
  [
      testCase "Parses Tuple" $ (queryTokens "/who/ is a husky" >>= parseMaybe query)
        @?= (Just (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]))

    , testCase "Parses And" $ (queryTokens "/who/ is a husky and (who) is cool" >>= parseMaybe query)
        @?= (Just (EQueryAnd (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")])))

    , testCase "Parses Or" $ (queryTokens "/who/ is a husky or (who) is cool" >>= parseMaybe query)
        @?= (Just (EQueryOr (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")])))

    , testCase "Parses When Clause" $ (queryTokens "When /who/ is a husky or (who) is cool { some gnarly C code }" >>= parseMaybe foxtalkWhen)
        @?= (Just (EWhen
                    (EQueryOr (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                    [BCodeLine " some gnarly C code "]))

    , testCase "Parses When Clause with Nested Bod" $
        (queryTokens "When /who/ is a husky or (who) is cool { some {gnarlier} C code }"
            >>= parseMaybe foxtalkWhen)
                @?= (Just (EWhen
                            (EQueryOr
                                (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                            [BCodeLine " some {gnarlier} C code "]))

    , testCase "Parses When Clause with Claims" $
        (queryTokens "When /who/ is a husky or (who) is cool{\n some {gnarlier} C code; \nClaim (who) is a dog\n}"
            >>= parseMaybe foxtalkWhen)
                @?= (Just (EWhen
                            (EQueryOr
                                (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                            [ BCodeLine "" -- first newline after {
                            , BCodeLine " some {gnarlier} C code; "
                            , BFoxtalkExpr (EClaim [VVarBinding "who", VLit (LSymbol "is"), VLit (LSymbol "a"), VLit (LSymbol "dog")])
                            ]))

    , testCase "Parses Forall Clause with Nested Bod" $
        (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }"
            >>= parseMaybe foxtalkForall)
                @?= (Just (EForAll "huskies"
                            (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                            [BCodeLine " some {gnarlier} C code "]))

    , testGroup "Top-level Parser" [
          testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]])

        , testCase "Parses When Clause" $
            (queryTokens "When /who/ is a husky or (who) is cool { some gnarly C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EWhen
                                (EQueryOr (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                                [BCodeLine " some gnarly C code "]])

        , testCase "Parses When Clause with Nested Bod" $
            (queryTokens "When /who/ is a husky or (who) is cool { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EWhen
                                (EQueryOr
                                    (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                    (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                                [BCodeLine " some {gnarlier} C code "]])

        , testCase "Parses Forall Clause with Nested Bod" $
            (queryTokens "ForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EForAll "huskies"
                                (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                [BCodeLine " some {gnarlier} C code "]])

        , testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]])

        , testCase "Parses Everything together" $
            (queryTokens "Claim lexi is a husky\nForAll /huskies/ When /who/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= (Just [
                        EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")],
                        EForAll "huskies"
                                (EQueryTuple [VVarIntro "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                [BCodeLine " some {gnarlier} C code "]])
    ]
  ]