module Parsing (parsingTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Tokenize (
    queryTokens
  , FoxtalkType (..)
  , QueryLiteral (..)
  , QueryToken(..)
  , handlerBody
  )

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
    testGroup "Literals" [
        testCase "Sym 1"           $ queryTokens "a" @?= Just [TLit (LSymbol "a")]
      , testCase "Sym 1"           $ queryTokens "fox" @?= Just [TLit (LSymbol "fox")]
      , testCase "Sym 3"           $ queryTokens "foxorlexi" @?= Just [TLit (LSymbol "foxorlexi")]
      , testCase "Int"             $ queryTokens "1234" @?= Just [TLit (LI64 1234)]
      , testCase "Negative Int"    $ queryTokens "-4321" @?= Just [TLit (LI64 (-4321))]
      , testCase "Unsigned Int"    $ queryTokens "88899u" @?= Just [TLit (LU64 88899)]
      , testCase "Double"          $ queryTokens "14.89" @?= Just [TLit (LDouble 14.89)]
      , testCase "Negative Double" $ queryTokens "-98.41" @?= Just [TLit (LDouble (-98.41))]
    ]

    , testGroup "Variable Introduction" [
        testCase "Untyped"            $ queryTokens "/lexi/" @?= Just [TUntypedVarIntro "lexi"]
      , testCase "Symbol"             $ queryTokens "/lexi:symbol/" @?= Just [TVarIntro TSymbol "lexi"]
      , testCase "Cptr"               $ queryTokens "/lexi:ptr/" @?= Just [TVarIntro TCptr "lexi"]
      , testCase "U64"                $ queryTokens "/lexi:u64/" @?= Just [TVarIntro TU64 "lexi"]
      , testCase "I64"                $ queryTokens "/lexi:i64/" @?= Just [TVarIntro TI64 "lexi"]
      , testCase "Double"             $ queryTokens "/lexi:double/" @?= Just [TVarIntro TDouble "lexi"]
      , testCase "Bytes"              $ queryTokens "/lexi:bytes/" @?= Just [TVarIntro TBytes "lexi"]
      , testCase "Fails with Spaces"  $ queryTokens "/test with spaces:symbol/" @?= Nothing
    ]

    , testCase "VarBinding 1" $ queryTokens "(lexi)" @?= Just [TVarBinding "lexi"]

    , testCase "VarBinding 2" $ queryTokens "(test with spaces)" @?= Just [TLParen, TLit (LSymbol "test"), TLit (LSymbol "with"), TLit (LSymbol "spaces"), TRParen]

    , testCase "BoundLit 1" $ queryTokens "/shape/@circle" @?= Just [TVarIntroLit TSymbol "shape" (LSymbol "circle")]

    , testCase "Tokens 1" $ queryTokens "/shape/@rectangle with x /x:double/ or /shape/@circle with r /r:u64/"
        @?= Just [TVarIntroLit TSymbol "shape" (LSymbol "rectangle")
                 , TLit (LSymbol "with"), TLit (LSymbol "x")
                 , TVarIntro TDouble "x", TOr, TVarIntroLit TSymbol "shape" (LSymbol "circle")
                 , TLit (LSymbol "with"), TLit (LSymbol "r"), TVarIntro TU64 "r"]

    , testCase "Tokens 2" $ queryTokens "((you) is a rectangle with x /x:double/) and ((you) has color /c:symbol/)"
        @?= Just ([TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "is a rectangle with x")
              ++ [TVarIntro TDouble "x", TRParen, TAnd, TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "has color")
              ++ [TVarIntro TSymbol "c", TRParen])

    , testCase "When" $ queryTokens "When bonk"
        @?= Just [TWhen, TLit (LSymbol "bonk")]

    , testCase "Handler Body 0" $ parseMaybe handlerBody "{ this is some c code or whatever }"
                                                @?= Just " this is some c code or whatever "

    , testCase "Handler Body 1" $ parseMaybe handlerBody "{ this is {some c} code or whatever }"
                                                @?= Just " this is {some c} code or whatever "

    , testCase "Handler Body 2" $ queryTokens "When bonk { this is some c code or whatever }"
        @?= Just [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some c code or whatever "]

    , testCase "Handler Body 3" $ queryTokens "When bonk { this is some for(;;) { gnarly} c code or whatever}"
        @?= Just [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some for(;;) { gnarly} c code or whatever"]

    , testCase "Handler Body 3 with initial  whitespace" $ queryTokens "     When bonk { this is some for(;;) { gnarly} c code or whatever}"
        @?= Just [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some for(;;) { gnarly} c code or whatever"]

    , testCase "Tokenizes Forall Clause with Nested Bod" $
        queryTokens "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
            @?= Just [TForAll, TUntypedVarIntro "huskies", TWhen, TVarIntro TSymbol "who"
                      , TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")
                      , THandlerBody " some {gnarlier} C code "]

    , testCase "Tokenizes Forall Clause with Nested Bod and initial whitespace" $
        queryTokens "      ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
            @?= Just [TForAll, TUntypedVarIntro "huskies", TWhen, TVarIntro TSymbol "who"
                      , TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")
                      , THandlerBody " some {gnarlier} C code "]

    , testCase "Tokenizes top-level Claim" $
        queryTokens "Claim lexi is a husky"
            @?= Just [TClaim, TLit (LSymbol "lexi"), TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")]

    , testCase "Tokenizes top-level Claim with beginning whitespace" $
        queryTokens "      Claim lexi is a husky"
            @?= Just [TClaim, TLit (LSymbol "lexi"), TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")]

  ]


parserTests :: TestTree
parserTests = testGroup "Parser Tests"
  [
      testCase "Parses Tuple" $ (queryTokens "/who:symbol/ is a husky" >>= parseMaybe query)
        @?= Just (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])

    , testCase "Parses And" $ (queryTokens "/who:symbol/ is a husky and (who) is cool" >>= parseMaybe query)
        @?= Just (EQueryAnd (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))

    , testCase "Parses Or" $ (queryTokens "/who:symbol/ is a husky or (who) is cool" >>= parseMaybe query)
        @?= Just (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))

    , testCase "Parses When Clause" $ (queryTokens "When /who:symbol/ is a husky or (who) is cool { some gnarly C code }" >>= parseMaybe foxtalkWhen)
        @?= Just (EWhen
                    (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                    [BCodeLine " some gnarly C code "])

    , testCase "Parses When Clause with Nested Bod" $
        (queryTokens "When /who:symbol/ is a husky or (who) is cool { some {gnarlier} C code }"
            >>= parseMaybe foxtalkWhen)
                @?= Just (EWhen
                            (EQueryOr
                                (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                            [BCodeLine " some {gnarlier} C code "])

    , testCase "Parses When Clause with Claims" $
        (queryTokens "When /who:symbol/ is a husky or (who) is cool{\n some {gnarlier} C code; \n      Claim (who) is a dog\n}"
            >>= parseMaybe foxtalkWhen)
                @?= Just (EWhen
                            (EQueryOr
                                (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                            [ BCodeLine "" -- first newline after {
                            , BCodeLine " some {gnarlier} C code; "
                            , BFoxtalkExpr (EClaim [VVarBinding "who", VLit (LSymbol "is"), VLit (LSymbol "a"), VLit (LSymbol "dog")])
                            ])

    -- TODO: Make this pass, also actually good errors.
    -- , testCase "Does not parse When Clause with BAD Claims" $
    --     (queryTokens "When /who:symbol/ is a husky or (who) is cool{\n some {gnarlier} C code; \nClaim /who:symbol/ is a dog\n}"
    --         >>= parseMaybe foxtalkWhen)
    --             @?= Nothing

    , testCase "Parses Forall Clause with Nested Bod" $
        (queryTokens "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
            >>= parseMaybe foxtalkForall)
                @?= Just (EForAll "huskies"
                            (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                            [BCodeLine " some {gnarlier} C code "])

    , testGroup "Top-level Parser" [
          testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]]

        , testCase "Parses When Clause" $
            (queryTokens "When /who:symbol/ is a husky or (who) is cool { some gnarly C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [EWhen
                                (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                                [BCodeLine " some gnarly C code "]]

        , testCase "Parses When Clause with Nested Bod" $
            (queryTokens "When /who:symbol/ is a husky or (who) is cool { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [EWhen
                                (EQueryOr
                                    (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                    (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                                [BCodeLine " some {gnarlier} C code "]]

        , testCase "Parses Forall Clause with Nested Bod" $
            (queryTokens "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [EForAll "huskies"
                                (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                [BCodeLine " some {gnarlier} C code "]]

        , testCase "Parses top-level Claim" $
            (queryTokens "Claim lexi is a husky"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]]

        , testCase "Parses Everything together" $
            (queryTokens "Claim lexi is a husky\nForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= Just [
                        EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")],
                        EForAll "huskies"
                                (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                                [BCodeLine " some {gnarlier} C code "]]

        -- TODO: Better Error Messaging
        , testCase "No Type allowed in ForAll var" $
            (queryTokens "Claim lexi is a husky\nForAll /huskies:symbol/ When /who:symbol/ is a husky { some {gnarlier} C code }"
                >>= parseMaybe foxtalkProgram)
                    @?= Nothing
    ]
  ]