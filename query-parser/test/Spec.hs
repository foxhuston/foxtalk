import Test.Tasty

import Parsing (parsingTests)

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Tests" $ [parsingTests]
