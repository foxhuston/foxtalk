{-# LANGUAGE OverloadedStrings #-}

module Lib
    ( someFunc
    ) where

someFunc :: IO ()
someFunc = print "bonk"
    -- undefined
    --  print $
    --     -- unwords .
    --     --     map unparse <$>
    --             parse tokens "illumination /shape/@rectangle at x /x/ y /y/ width /width/ height /height/"
    --             -- parseOnly tokens "(illumination /shape/@rectangle at x /x/ y /y/ width /width/ height /height/) or (illumination /shape/@circle    at x /x/ y /y/ radius /r/)"
