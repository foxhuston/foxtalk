import Test.Tasty

import Parsing (parsingTests)
import CodeGeneration (codeGenerationTests)

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Tests" $ [parsingTests, codeGenerationTests]
