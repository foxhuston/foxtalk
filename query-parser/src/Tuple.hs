{-# LANGUAGE ScopedTypeVariables #-}

module Tuple (Tuple (..), TupleNoun(..)) where
import Data.Int (Int64)
import Data.Word (Word64)
import Data.Array.Byte (ByteArray)
import Data.List (intercalate)

data TupleNoun =
    Query
  | Symbol String
  | CPtr Word64
  | U64 Word64
  | I64 Int64
  | Bytes ByteArray
  | Prefix
  deriving Show


newtype Tuple = Tuple [TupleNoun]

instance Show Tuple where
  show (Tuple nouns) =
    let nstrs :: [String] = map show nouns
    in "<" ++ intercalate ", " nstrs ++ ">"