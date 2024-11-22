module Main (main) where

import Parse (parseProgram)
import Codegen (genHandleBody)

main :: IO ()
main =
  let (Just progExpr) = parseProgram "When /who:symbol/ is a husky { Claim (who) is cool }"
      (Right bod)     = genHandleBody 0 $ progExpr !! 0
  in putStrLn $ unlines bod
