module CodeGeneration (codeGenerationTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , FoxtalkExpr (..)
  , parseProgram
  )

import Foxtalk (
    foxtalkExprToPosition
  )


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [queryGenerationTests]


queryGenerationTests :: TestTree
queryGenerationTests = testGroup "Query Tuple Generation Tests" [
      testCase "To Position" $
        (map foxtalkExprToPosition) <$> (parseProgram "Claim /x/ is a /foo/") @?=
          Just [EClaim [VVarIntro 0
                       , VLit (LSymbol "is"), VLit (LSymbol "a")
                       , VVarIntro 3]]
  ]