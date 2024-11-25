module Main (main) where

import System.Environment (getArgs)

import Parse (parseProgram)
import Codegen (genProg, runFoxtalkCodegen)

main :: IO ()
main =
  do
    args <- getArgs
    case args of
      [handlerName, file] ->
        do
          progSource <- readFile file
          case parseProgram file progSource of
            (Right progExprs) ->
              case runFoxtalkCodegen $ genProg handlerName progExprs of
                (Right output) -> putStrLn output
                (Left err) -> putStrLn $ "Error in CodeGen: " ++ err
            Left err -> putStrLn $ "Error in parseProgram: " ++ err

      _ -> putStrLn "usage: foxtalkc <handler name> <source path>"
