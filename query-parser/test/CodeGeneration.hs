module CodeGeneration (codeGenerationTests) where

import Test.Tasty
import Test.Tasty.HUnit

import Parse (parseProgram)
import Foxtalk (
  queryValueToTupleEntry
  )


codeGenerationTests :: TestTree
codeGenerationTests = testGroup "Code Generation Tests" [{- queryGenerationTests-}]


-- queryGenerationTests :: TestTree
-- queryGenerationTests = testGroup "Query Tuple Generation Tests" [
--       testCase "QueryValue Symbol" $
--         (parseProgram "Claim foo" >>= undefined) @?= undefined
--   ]