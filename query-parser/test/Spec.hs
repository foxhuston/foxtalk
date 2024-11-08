import Test.Tasty
import Test.Tasty.HUnit
import Test.Tasty.QuickCheck as QC

import Data.List (sort)

import Token (tokens, QueryToken(..))

main :: IO ()
main = defaultMain tests


tests :: TestTree
tests = testGroup "Tests" [unitTests {-, qcProps -}]

-- qcProps :: TestTree
-- qcProps = testGroup "(checked by quickcheck)"
--   [ ]


unitTests :: TestTree
unitTests = testGroup "Unit tests"
  [
      testCase "Ident 1" $ tokens "a" @?= Just [TIdent "a"]
    , testCase "Ident 1" $ tokens "fox" @?= Just [TIdent "fox"]
    , testCase "Binding 1" $ tokens "/lexi/" @?= Just [TBinding "lexi"]
    , testCase "Binding 2" $ tokens "/test with spaces/" @?= Nothing
    , testCase "BoundLit 1" $ tokens "/shape/@circle" @?= Just [TBoundLit "shape" "circle"]
    , testCase "tokens" $ tokens "/shape/@rectangle with x /x/ or /shape/@circle with r /r/"
        @?= Just [TBoundLit "shape" "rectangle", TIdent "with", TIdent "x", TBinding "x", TOr, TBoundLit "shape" "circle", TIdent "with", TIdent "r", TBinding "r"]
  ]