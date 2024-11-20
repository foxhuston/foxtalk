module CodeGeneration (codeGenerationTests) where

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

import Foxtalk (
    foxtalkExprToPosition
  )


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [queryGenerationTests]


queryGenerationTests :: TestTree
queryGenerationTests = testGroup "Query Tuple Generation Tests" [
      testCase "To Position" $
        (map foxtalkExprToPosition) <$> (parseProgram "Claim /x:u64/ is a /foo:symbol/") @?=
          Just [EClaim [VVarIntro TU64 0
                       , VLit (LSymbol "is"), VLit (LSymbol "a")
                       , VVarIntro TSymbol 3]]
  ]