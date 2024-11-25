module CodeGenTests (codeGenerationTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Parse (
    FoxtalkType(..)
  , QueryLiteral(..)
  , QueryValue(..)
  , QueryExpr(..)
  , FoxtalkExpr(..)
  , parseProgram
  )

import Codegen (
    exprToHandlerQuery
  , exprToClaim
  , foxtalkExprToPosition
  , foxtalkExprToLookup
  , queryValuePosToLookup
  , queryExprToLookup
  , queryExprToLookupGuard
  , genHandleBody
  )


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [
    queryGenerationTests
  -- , handlerGenerationTests
  -- , claimGenerationTests
  ]

withSingleFoxtalkExpr :: (Eq a, Show a) => (FoxtalkExpr String -> Either String a) -> String -> a -> Assertion
withSingleFoxtalkExpr f src expected =
  case parseProgram src "CodeGenTests" of
    Right [singleExpr] ->
      case f singleExpr of
        Right out -> out @?= expected
        Left err  -> assertFailure err
    Right _  -> assertFailure "Too many expressions in parser output!"
    Left err -> assertFailure err

queryeq :: String -> [String] -> Assertion
queryeq = withSingleFoxtalkExpr exprToHandlerQuery

poseq :: String -> FoxtalkExpr (Int, String) -> Assertion
poseq = withSingleFoxtalkExpr (pure . foxtalkExprToPosition)


queryGenerationTests :: TestTree
queryGenerationTests = testGroup "Query Tuple Generation Tests" [
  testCase "Single To Query" $
    "When /who:symbol/ is a husky {}" `queryeq` ["claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}});"]

  , testCase "Disjunction To Query" $
    "When /who:symbol/ is a husky or /who:symbol/ is a kitten {}"
      `queryeq` [
          "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}});"
        , "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"kitten\"}}});"
        ]

  , testCase "Conjunction To Query" $
    "When /who:symbol/ is a husky and /who:symbol/ is a kitten {}"
      `queryeq` [
          "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}});"
        , "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"kitten\"}}});"
        ]

  -- TODO
  -- , testCase "Claims generate no queries" $
  --   let parsed = parseProgram  "Claim /who:symbol/ is a husky"
  --       handlerQuery = parsed >>= exprToHandlerQuery . (!!0)
  --   in handlerQuery @?= Nothing
  ]

handlerGenerationTests :: TestTree
handlerGenerationTests = testGroup "Handler Generation Tests" [
    testCase "To Position" $
      "Claim /x:u64/ is a /foo:symbol/" `poseq`
        EClaim [VVarIntro TU64 (0, "x")
                      , VLit (LSymbol "is"), VLit (LSymbol "a")
                      , VVarIntro TSymbol (3, "foo")]

  , testGroup "Lookups" [
      testCase "Query Value" $
        queryValuePosToLookup "res" (VVarIntro TU64 (0, "x"))
          @?= Just "auto x = res.at<uint64_t>(0)"

    , testCase "Claim Expr" $
        (foxtalkExprToLookup "res" . (!!0) <$> parseProgram "Claim /x:u64/") @?=
          Just []

    , testCase "When Expr" $
        (foxtalkExprToLookup "res" . (!!0) <$> parseProgram "When /x:u64/ {}") @?=
          Just [
              "auto x = res.at<uint64_t>(0)"
          ]

    , testCase "When Expr 2" $
        (foxtalkExprToLookup "res" . (!!0) <$> parseProgram "When /x:u64/ is a /foo:symbol/ {}") @?=
          Just [
            "auto x = res.at<uint64_t>(0)"
          , "auto foo = res.at<std::string>(3)"
          ]
    ]

--     -- , testGroup "Guards" [
--     --     testCase "Claim Expr" $
--     --       (foxtalkExprToLookupGuard . (!!0) <$> parseProgram "Claim /x:u64/") @?=
--     --         Just []

--     --   , testCase "When Expr" $
--     --       (foxtalkExprToLookupGuard . (!!0) <$> parseProgram "When /x:u64/ {}") @?=
--     --         Just [
--     --             "if !(x.has_value()) { return; }"
--     --         ]

--     --   , testCase "When Expr 2" $
--     --       (foxtalkExprToLookupGuard . (!!0) <$> parseProgram "When /x:u64/ is a /foo:symbol/ {}") @?=
--     --         Just [
--     --           "if !(x.has_value()) { return; }"
--     --         , "if !(foo.has_value()) { return; }"
--     --         ]
--     --   ]


--     , testGroup "Handle Body" [
--         testCase "Init" $
--           (genHandleBody 0 . (!!0) <$> parseProgram "When /who:symbol/ is a husky { Claim (who) is cool }")
--             @?= Just [
--                 "void handle(const std::vector<Tuple> &queryResults0) override"
--               , "{"
--               , "auto who = queryResults0.at<std::string>(0);"
--               , ""
--               , "if(!who.has_value()) { return; }"
--               , "}"
--               ]
--     ]

--   ]

-- claimGenerationTests :: TestTree
-- claimGenerationTests = testGroup "Claim Generation Tests" [
--   testCase "All Symbols" $
--     let parsed = parseProgram  "Claim lexi is a husky"
--         handlerQuery = parsed >>= exprToClaim . (!!0)
--     in handlerQuery @?= Just "claim({{{\"lexi\"}, {\"is\"}, {\"a\"}, {\"husky\"}}})"

--   , testCase "Some Numbers" $
--     let parsed = parseProgram  "Claim camera has width 3 height 4"
--         handlerQuery = parsed >>= exprToClaim . (!!0)
--     in handlerQuery @?= Just "claim({{{\"camera\"}, {\"has\"}, {\"width\"}, {3}, {\"height\"}, {4}}})"

--   , testCase "Variable Binding" $
--     let parsed = parseProgram  "Claim (who) is cool"
--         handlerQuery = parsed >>= exprToClaim . (!!0)
--     in handlerQuery @?= Just "claim({{{who}, {\"is\"}, {\"cool\"}}})"
  ]
