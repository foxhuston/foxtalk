module Tuple (Tuple (..)) where
import Data.Int (Int64)
import Data.Word (Word64)
import Data.Array.Byte (ByteArray)

data Tuple =
    Query
  | Symbol String
  | CPtr Word64
  | U64 Word64
  | I64 Int64
  | Bytes ByteArray
  | Prefix
  deriving Show


