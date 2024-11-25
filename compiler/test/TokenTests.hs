module TokenTests (tokenTests) where

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
    , parseProgram
    )

import Text.Megaparsec (runParser, errorBundlePretty)

tokeq :: String -> [QueryToken String] -> Assertion
tokeq src expectedToks =
  case queryTokens "Parsing Tests" src of
    Right returnedToks -> returnedToks @?= expectedToks
    Left errs          -> assertFailure errs

queryeq :: String -> QueryExpr String -> Assertion
queryeq src expected =
  case queryTokens "ParsingTests" src of
    Right toks ->
      case runParser query "Parsing Tests" toks of
        Right returned -> returned @?= expected
        Left _         -> assertFailure "Error in QueryEq Parser"
    Left _         -> assertFailure "Error in QueryEq Tokenization"

parseq :: String -> [FoxtalkExpr String] -> Assertion
parseq src expectedExprs =
  case parseProgram "Parsing Tests" src of
    Right returnedExprs -> returnedExprs @?= expectedExprs
    Left errs           -> assertFailure errs

handlerbodyeq :: String -> String -> Assertion
handlerbodyeq src expected =
  case runParser handlerBody "Parsing Tests" src of
    Right returned -> returned @?= expected
    Left _         -> assertFailure "Handler body eq failure"

tokenTests :: TestTree
tokenTests = testGroup "Token tests"
  [
    testGroup "Literals" [
        testCase "Sym 1"           $ "a"         `tokeq` [TLit (LSymbol "a")]
      , testCase "Sym 1"           $ "fox"       `tokeq` [TLit (LSymbol "fox")]
      , testCase "Sym 3"           $ "foxorlexi" `tokeq` [TLit (LSymbol "foxorlexi")]
      , testCase "Int"             $ "1234"      `tokeq` [TLit (LI64 1234)]
      , testCase "Negative Int"    $ "-4321"     `tokeq` [TLit (LI64 (-4321))]
      , testCase "Unsigned Int"    $ "88899u"    `tokeq` [TLit (LU64 88899)]
      , testCase "Double"          $ "14.89"     `tokeq` [TLit (LDouble 14.89)]
      , testCase "Negative Double" $ "-98.41"    `tokeq` [TLit (LDouble (-98.41))]
    ]

    , testGroup "Variable Introduction" [
        testCase "Untyped"            $ "/lexi/"        `tokeq` [TUntypedVarIntro  "lexi"]
      , testCase "Underscore"         $ "/lexi_husky/"  `tokeq` [TUntypedVarIntro  "lexi_husky"]
      , testCase "Symbol"             $ "/lexi:symbol/" `tokeq` [TVarIntro TSymbol "lexi"]
      , testCase "Cptr"               $ "/lexi:ptr/"    `tokeq` [TVarIntro TCptr   "lexi"]
      , testCase "U64"                $ "/lexi:u64/"    `tokeq` [TVarIntro TU64    "lexi"]
      , testCase "I64"                $ "/lexi:i64/"    `tokeq` [TVarIntro TI64    "lexi"]
      , testCase "Double"             $ "/lexi:double/" `tokeq` [TVarIntro TDouble "lexi"]
      , testCase "Bytes"              $ "/lexi:bytes/"  `tokeq` [TVarIntro TBytes  "lexi"]
      , testCase "Captured Prefix"    $ "/lexi:.../"    `tokeq` [TVarIntro TRest   "lexi"]
      -- TODO!
      -- , testCase "Fails with Spaces"  $ "/test with spaces:symbol/" `tokeqNothing
    ]

    , testGroup "Keywords" [
        testCase "When"       $ "When"      `tokeq` [TWhen]
      , testCase "Claim"      $ "Claim"     `tokeq` [TClaim]
      , testCase "ForAll"     $ "ForAll"    `tokeq` [TForAll]
      , testCase "or"         $ "or"        `tokeq` [TOr]
      , testCase "and"        $ "and"       `tokeq` [TAnd]
      , testCase "Locals"     $ "Locals"    `tokeq` [TLocals]
      , testCase "Poll"       $ "Poll"      `tokeq` [TPoll]
      , testCase "FreeTuple"  $ "FreeTuple" `tokeq` [TFreeTuple]
      , testCase "..."        $ "..."       `tokeq` [TPrefix]
    ]

    , testCase "VarBinding 1" $ "(lexi)" `tokeq` [TVarBinding "lexi"]

    , testCase "VarBinding 2" $ "(test with spaces)" `tokeq` [TLParen, TLit (LSymbol "test"), TLit (LSymbol "with"), TLit (LSymbol "spaces"), TRParen]

    , testCase "BoundLit 1" $ "/shape/@circle" `tokeq` [TVarIntroLit TSymbol "shape" (LSymbol "circle")]

    , testCase "Tokens 1" $ "/shape/@rectangle with x /x:double/ or /shape/@circle with r /r:u64/"
        `tokeq` [TVarIntroLit TSymbol "shape" (LSymbol "rectangle")
                 , TLit (LSymbol "with"), TLit (LSymbol "x")
                 , TVarIntro TDouble "x", TOr, TVarIntroLit TSymbol "shape" (LSymbol "circle")
                 , TLit (LSymbol "with"), TLit (LSymbol "r"), TVarIntro TU64 "r"]

    , testCase "Tokens 2" $ "((you) is a rectangle with x /x:double/) and ((you) has color /c:symbol/)"
        `tokeq` ([TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "is a rectangle with x")
              ++ [TVarIntro TDouble "x", TRParen, TAnd, TLParen, TVarBinding "you"]
              ++ map (TLit . LSymbol) (words "has color")
              ++ [TVarIntro TSymbol "c", TRParen])

    , testCase "When" $ "When bonk"
        `tokeq` [TWhen, TLit (LSymbol "bonk")]

    , testCase "Handler Body 0" $ "{ this is some c code or whatever }"
                                    `handlerbodyeq` " this is some c code or whatever "

    , testCase "Handler Body 1" $ "{ this is {some c} code or whatever }"
                                    `handlerbodyeq` " this is {some c} code or whatever "

    , testCase "Handler Body 2" $ "When bonk { this is some c code or whatever }"
        `tokeq` [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some c code or whatever "]

    , testCase "handler body 3" $ "When bonk { this is some for(;;) { gnarly} c code or whatever}"
        `tokeq` [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some for(;;) { gnarly} c code or whatever"]

    , testCase "Handler Body 3 with initial  whitespace" $ "     When bonk { this is some for(;;) { gnarly} c code or whatever}"
        `tokeq` [TWhen, TLit (LSymbol "bonk"), THandlerBody " this is some for(;;) { gnarly} c code or whatever"]

    , testCase "Tokenizes Forall Clause with Nested Bod" $
        "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
            `tokeq` [TForAll, TUntypedVarIntro "huskies", TWhen, TVarIntro TSymbol "who"
                      , TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")
                      , THandlerBody " some {gnarlier} C code "]

    , testCase "Tokenizes Forall Clause with Nested Bod and initial whitespace" $
        "      ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
            `tokeq` [TForAll, TUntypedVarIntro "huskies", TWhen, TVarIntro TSymbol "who"
                      , TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")
                      , THandlerBody " some {gnarlier} C code "]

    , testCase "Tokenizes top-level Claim" $
        "Claim lexi is a husky"
            `tokeq` [TClaim, TLit (LSymbol "lexi"), TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")]

    , testCase "Tokenizes top-level Claim with beginning whitespace" $
        "      Claim lexi is a husky"
            `tokeq` [TClaim, TLit (LSymbol "lexi"), TLit (LSymbol "is"), TLit (LSymbol "a"), TLit (LSymbol "husky")]

  ]


parserTests :: TestTree
parserTests = testGroup "Parser Tests"
  [
      testCase "Parses Tuple" $ "/who:symbol/ is a husky"
        `queryeq` (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])

    , testCase "Parses And" $ "/who:symbol/ is a husky and (who) is cool"
        `queryeq` (EQueryAnd (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))

    , testCase "Parses Or" $ "/who:symbol/ is a husky or (who) is cool"
        `queryeq` (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))

    , testCase "Parses When Clause" $ "When /who:symbol/ is a husky or (who) is cool { some gnarly C code }"
        `parseq` [EWhen
                    (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                    [BCodeLine " some gnarly C code "]]

    , testCase "Parses When Clause with Nested Bod" $ "When /who:symbol/ is a husky or (who) is cool { some {gnarlier} C code }"
        `parseq` [EWhen
                    (EQueryOr
                        (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                        (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                    [BCodeLine " some {gnarlier} C code "]]

    , testCase "Parses When Clause with Claims" $ "When /who:symbol/ is a husky or (who) is cool{\n some {gnarlier} C code; \n      Claim (who) is a dog\n}"
        `parseq` [EWhen
                    (EQueryOr
                        (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                        (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                    [ BCodeLine "" -- first newline after {
                    , BCodeLine " some {gnarlier} C code; "
                    , BFoxtalkExpr (EClaim [VVarBinding "who", VLit (LSymbol "is"), VLit (LSymbol "a"), VLit (LSymbol "dog")])
                    ]]

    -- TODO: Make this pass, also actually good errors.
    -- , testCase "Does not parse When Clause with BAD Claims" $
    --     (queryTokens "When /who:symbol/ is a husky or (who) is cool{\n some {gnarlier} C code; \nClaim /who:symbol/ is a dog\n}"
    --         >>= parseMaybe foxtalkWhen)
    --             @?= Nothing

    , testCase "Parses Forall Clause with Nested Bod" $
        "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
          `parseq` [EForAll "huskies"
                      (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                      [BCodeLine " some {gnarlier} C code "]]

    , testGroup "Top-level Parser" [
          testCase "Parses top-level Claim" $
            "Claim lexi is a husky"
              `parseq` [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]]

        , testCase "Parses When Clause" $
            "When /who:symbol/ is a husky or (who) is cool { some gnarly C code }"
              `parseq` [EWhen
                          (EQueryOr (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]) (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                          [BCodeLine " some gnarly C code "]]

        , testCase "Parses When Clause with Nested Bod" $
            "When /who:symbol/ is a husky or (who) is cool { some {gnarlier} C code }"
              `parseq` [EWhen
                          (EQueryOr
                              (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                              (EQueryTuple [VVarBinding "who",VLit (LSymbol "is"),VLit (LSymbol "cool")]))
                          [BCodeLine " some {gnarlier} C code "]]

        , testCase "Parses Forall Clause with Nested Bod" $
            "ForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
              `parseq` [EForAll "huskies"
                          (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                          [BCodeLine " some {gnarlier} C code "]]

        , testCase "Parses top-level Claim" $
            "Claim lexi is a husky"
              `parseq` [EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")]]

        , testCase "Parses Everything together" $
            "Claim lexi is a husky\nForAll /huskies/ When /who:symbol/ is a husky { some {gnarlier} C code }"
              `parseq` [
                  EClaim [VLit (LSymbol "lexi"),VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")],
                  EForAll "huskies"
                          (EQueryTuple [VVarIntro TSymbol "who",VLit (LSymbol "is"),VLit (LSymbol "a"),VLit (LSymbol "husky")])
                          [BCodeLine " some {gnarlier} C code "]]

--         -- TODO: Better Error Messaging
--         -- , testCase "No Type allowed in ForAll var" $
--         --     (queryTokens "Claim lexi is a husky\nForAll /huskies:symbol/ When /who:symbol/ is a husky { some {gnarlier} C code }"
--         --         >>= parseMaybe foxtalkProgram)
--         --             @?= Nothing
    ]
  ]