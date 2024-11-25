module ParsingTests (parsingTests) where

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

parsingTests :: TestTree
parsingTests = testGroup "Parser Tests"
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