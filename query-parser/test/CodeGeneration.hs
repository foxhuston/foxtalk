module CodeGeneration (codeGenerationTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Parse (parseProgram)


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [queryGenerationTests]


queryGenerationTests :: TestTree
queryGenerationTests = testGroup "Query Tuple Generation Tests" [
      testCase "Ident 1" $ 1 @?= 2
  ]