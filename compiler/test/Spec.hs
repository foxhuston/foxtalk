import Test.Tasty

import TokenTests (tokenTests)
import ParsingTests (parsingTests)
import CodeGenTests (codeGenerationTests)

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Tests" $ [tokenTests, parsingTests, codeGenerationTests]
