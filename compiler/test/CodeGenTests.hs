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
  )


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [
  queryGenerationTests
  , claimGenerationTests
  ]

queryGenerationTests :: TestTree
queryGenerationTests = testGroup "Query Tuple Generation Tests" [
  testCase "Single To Query" $
    let parsed = parseProgram  "When /who:symbol/ is a husky {}"
        handlerQuery = parsed >>= (exprToHandlerQuery . (!!0))
    in handlerQuery @?= Just ["claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}})"]

  , testCase "Disjunction To Query" $
    let parsed = parseProgram  "When /who:symbol/ is a husky or /who:symbol/ is a kitten {}"
        handlerQuery = parsed >>= (exprToHandlerQuery . (!!0))
    in handlerQuery @?= Just [
        "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}})"
      , "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"kitten\"}}})"
      ]

  , testCase "Conjunction To Query" $
    let parsed = parseProgram  "When /who:symbol/ is a husky and /who:symbol/ is a kitten {}"
        handlerQuery = parsed >>= (exprToHandlerQuery . (!!0))
    in handlerQuery @?= Just [
        "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"husky\"}}})"
      , "claim({{TupleNoun::query(), {\"is\"}, {\"a\"}, {\"kitten\"}}})"
      ]

  , testCase "Claims generate no queries" $
    let parsed = parseProgram  "Claim /who:symbol/ is a husky"
        handlerQuery = parsed >>= (exprToHandlerQuery . (!!0))
    in handlerQuery @?= Nothing
  ]

handlerGenerationTests :: TestTree
handlerGenerationTests = testGroup "Handler Generation Tests" [
  testCase "To Position" $
    (map foxtalkExprToPosition) <$> (parseProgram "Claim /x:u64/ is a /foo:symbol/") @?=
      Just [EClaim [VVarIntro TU64 (0, "x")
                    , VLit (LSymbol "is"), VLit (LSymbol "a")
                    , VVarIntro TSymbol (3, "foo")]]
  ]

claimGenerationTests :: TestTree
claimGenerationTests = testGroup "Claim Generation Tests" [
  testCase "All Symbols" $
    let parsed = parseProgram  "Claim lexi is a husky"
        handlerQuery = parsed >>= (exprToClaim . (!!0))
    in handlerQuery @?= Just "claim({{{\"lexi\"}, {\"is\"}, {\"a\"}, {\"husky\"}}})"

  , testCase "Some Numbers" $
    let parsed = parseProgram  "Claim camera has width 3 height 4"
        handlerQuery = parsed >>= (exprToClaim . (!!0))
    in handlerQuery @?= Just "claim({{{\"camera\"}, {\"has\"}, {\"width\"}, {3}, {\"height\"}, {4}}})"

  , testCase "Variable Binding" $
    let parsed = parseProgram  "Claim (who) is cool"
        handlerQuery = parsed >>= (exprToClaim . (!!0))
    in handlerQuery @?= Just "claim({{{who}, {\"is\"}, {\"cool\"}}})"
  ]
