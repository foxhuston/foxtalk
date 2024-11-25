module Codegen (
    exprToHandlerQuery
  , exprToClaim
  , foxtalkExprToLookup

  , genProg
  , genHandleBody
  , genQueryBody
  , runFoxtalkCodegen

  , disambiguateTuples -- for testing only
  , queryValueToTupleEntry -- for testing only
  , queryExprToLookup -- for testing only
  , queryExprToLookupGuard -- for testing only
  , foxtalkExprToPosition -- for testing only
  , queryValuePosToLookup -- for testing only
) where

import Data.List (intercalate, find, nub)

import Control.Monad.Except (throwError)
import Control.Monad.State (StateT, lift, get, gets, modify, execStateT)

import Parse (
    QueryLiteral (..)
  , QueryValue (..)
  , QueryExpr (..)
  , QueryExprF (..)
  , HandlerBodyLine (..)
  , FoxtalkExpr (..)
  , FoxtalkExprF (..)
  , FoxtalkType (..),
  )
import Data.Functor.Foldable (cata)
import Data.Maybe (mapMaybe)

type CExpr = String

----- Into Query -----
queryLitToTupleEntry :: QueryLiteral -> String
queryLitToTupleEntry (LSymbol l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LU64    l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LI64    l) = "{" ++ show l ++ "}"
queryLitToTupleEntry (LDouble l) = "{" ++ show l ++ "}"

queryValueToTupleEntry :: (QueryValue String) -> String
queryValueToTupleEntry (VPrefixMarker)      = "TupleNoun::prefix()"
queryValueToTupleEntry (VLit l)             = queryLitToTupleEntry l
queryValueToTupleEntry (VVarIntro _ _)      = "TupleNoun::query()"
queryValueToTupleEntry (VVarBinding s)      = "{" ++ s ++ "}"
queryValueToTupleEntry (VVarIntroLit _ _ l) = queryLitToTupleEntry l

intoClaims :: QueryExpr String -> Either String [CExpr]
intoClaims (EQueryTuple values) =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in return ["claim({{" ++ vs ++ "}});"]
intoClaims (EQueryAnd l r) =
  do
    ll <- intoClaims l
    rr <- intoClaims r
    return $ ll ++ rr
intoClaims (EQueryOr l r) =
  do
    ll <- intoClaims l
    rr <- intoClaims r
    return $ ll ++ rr

exprToHandlerQuery :: FoxtalkExpr String -> Either String [CExpr]
exprToHandlerQuery (EWhen q _)      = intoClaims q
exprToHandlerQuery (EForAll _ q _)  = intoClaims q
exprToHandlerQuery (EClaim _)       = Left "Cannot generate handler query from Claim"

----- Generate Claim -----
exprToClaim :: FoxtalkExpr String -> Either String CExpr
exprToClaim (EClaim values) =
  let vs = intercalate ", " $ map queryValueToTupleEntry values
  in return $ "claim({{" ++ vs ++ "}})"
exprToClaim (EWhen _ _)     = Left "Could not generate string from EWhen"
exprToClaim (EForAll _ _ _) = Left "Could not generate string from EForAll"

----- DISAMBIGUATE TUPLES -----
getQueryTupleValues :: forall a. QueryExpr a -> [[QueryValue a]]
getQueryTupleValues = cata go
  where
    go :: QueryExprF a [[QueryValue a]] -> [[QueryValue a]]
    go (EQueryTupleF exprs) = [exprs]
    go (EQueryAndF l r)     = l ++ r
    go (EQueryOrF l r)      = l ++ r

getTupleValues :: FoxtalkExpr a -> [[(Int, QueryValue a)]]
getTupleValues (EWhen q _)     = map (zip [0..]) $ getQueryTupleValues q
getTupleValues (EForAll _ q _) = map (zip [0..]) $ getQueryTupleValues q
getTupleValues (EClaim vs)     = [zip [0..] vs]

disambiguateTuples :: forall a. Eq a => FoxtalkExpr a -> Maybe [(Int, QueryValue a)]
disambiguateTuples expr = select tuples <$> find unambig [0..maxL]
  where
    tuples :: [[(Int, QueryValue a)]]
    tuples = getTupleValues expr

    trunc :: [[(Int, QueryValue a)]]
    trunc = map (take maxL) tuples

    select :: [[(Int, QueryValue a)]] -> Int -> [(Int, QueryValue a)]
    select ls idx = map (!!idx) ls

    maxL :: Int
    maxL = maximum $ map length tuples

    unambig :: Int -> Bool
    unambig idx = nub xs == xs
      where xs = select tuples idx

----- GENERATE LOCALS -----
foxtalkTypeToCType :: FoxtalkType -> String
foxtalkTypeToCType TSymbol  = "std::string"
foxtalkTypeToCType TCptr    = "void*"
foxtalkTypeToCType TU64     = "uint64_t"
foxtalkTypeToCType TI64     = "int64_t"
foxtalkTypeToCType TDouble  = "double"
foxtalkTypeToCType TBytes   = "std::vector<uint8_t>"
foxtalkTypeToCType TRest    = "std::vector<TupleNoun>"

generateLookupVar :: String -> FoxtalkType -> String -> Int -> CExpr
generateLookupVar queryResult TRest varName varIndex =
  "std::vector<TupleNoun> " ++ varName ++ "(" ++ queryResult ++ ".begin() + " ++ show varIndex ++
    ", " ++ queryResult ++ ".end());"
generateLookupVar queryResult t varName varIndex =
  "auto " ++ varName ++ " = " ++ queryResult
    ++ ".at<" ++ foxtalkTypeToCType t ++ ">(" ++ show varIndex ++ ")"

queryValuePosToLookup :: String -> QueryValue (Int, String) -> Maybe CExpr
queryValuePosToLookup _  (VPrefixMarker)                = Nothing
queryValuePosToLookup _  (VLit _)                       = Nothing
queryValuePosToLookup qr (VVarIntro t (idx, name))      = Just $ generateLookupVar qr t name idx
queryValuePosToLookup _  (VVarBinding _)                = Nothing
queryValuePosToLookup qr (VVarIntroLit t (idx, name) _) = Just $ generateLookupVar qr t name idx

queryExprToLookup :: String -> QueryExpr (Int, String) -> [CExpr]
queryExprToLookup qr = cata go
  where
    -- Tbh, I'm not sure this is better... It's pretty cool, though.
    go :: QueryExprF (Int, String) [CExpr] -> [CExpr]
    go (EQueryTupleF exprs) = mapMaybe (queryValuePosToLookup qr) exprs
    go (EQueryAndF l r) = l ++ r
    go (EQueryOrF l r) = l ++ r

foxtalkExprToLookup :: String -> FoxtalkExpr String -> [CExpr]
foxtalkExprToLookup qr = go . foxtalkExprToPosition
  where
    go (EWhen q _)     = queryExprToLookup qr q
    -- TODO: Correctly do ForAll
    go (EForAll _ _ _) = undefined
    go (EClaim _)      = []

----- LOOKUP GUARDS -----
queryExprToLookupGuard :: QueryExpr String -> [CExpr]
queryExprToLookupGuard =
   map (\(_, a) -> "if(!" ++ a ++ ".has_value()) { return; }") . queryExtractVarIntros

----- EXTRACT VAR INTROS -----
queryExtractVarIntros :: QueryExpr a -> [(FoxtalkType, a)]
queryExtractVarIntros = cata go
  where
    go :: QueryExprF a [(FoxtalkType, a)] -> [(FoxtalkType, a)]
    go (EQueryTupleF vs)  = filterVars vs
    go (EQueryAndF l r)   = l ++ r
    go (EQueryOrF l r)    = l ++ r

    filterVars :: [QueryValue a] -> [(FoxtalkType, a)]
    filterVars []                       = []
    filterVars (VVarIntro t a:vs)       = (t, a) : filterVars vs
    filterVars (VVarIntroLit t a _:vs)  = (t, a) : filterVars vs
    filterVars (_:vs)                   = filterVars vs

----- TO POSITON -----
queryValuesToPosition :: [QueryValue a] -> [QueryValue (Int, a)]
queryValuesToPosition = posn 0
  where
    posn :: Int -> [QueryValue a] -> [QueryValue (Int, a)]
    posn _ [] = []
    posn n (VPrefixMarker:xs)       = VPrefixMarker : posn (n + 1) xs
    posn n (VLit l:xs)              = VLit l : posn (n + 1) xs
    posn n (VVarIntro t a:xs)       = VVarIntro t (n, a) : posn (n + 1) xs
    posn n (VVarBinding a:xs)       = VVarBinding (n, a) : posn (n + 1) xs
    posn n (VVarIntroLit t a l:xs)  = VVarIntroLit t (n, a) l : posn (n + 1) xs

queryExprToPosition :: QueryExpr a -> QueryExpr (Int, a)
queryExprToPosition (EQueryTuple vs)  = EQueryTuple $ queryValuesToPosition vs
queryExprToPosition (EQueryAnd l r)   = EQueryAnd (queryExprToPosition l) (queryExprToPosition r)
queryExprToPosition (EQueryOr l r)    = EQueryOr (queryExprToPosition l) (queryExprToPosition r)

handlerBodyLineToPosition :: HandlerBodyLine a -> HandlerBodyLine (Int, a)
handlerBodyLineToPosition (BCodeLine code) = BCodeLine code
handlerBodyLineToPosition (BFoxtalkExpr ft) = BFoxtalkExpr $ foxtalkExprToPosition ft

foxtalkExprToPosition :: FoxtalkExpr a -> FoxtalkExpr (Int, a)
foxtalkExprToPosition (EWhen q h)     = EWhen (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EForAll a q h) = EForAll (-1, a) (queryExprToPosition q) (map handlerBodyLineToPosition h)
foxtalkExprToPosition (EClaim vs)     = EClaim $ queryValuesToPosition vs

handlerBodyToCExpr :: HandlerBodyLine String -> Either String [CExpr]
handlerBodyToCExpr (BCodeLine code) = pure [code]
handlerBodyToCExpr (BFoxtalkExpr (EClaim vs)) = intoClaims (EQueryTuple vs)
handlerBodyToCExpr (BFoxtalkExpr _) = pure ["// TODO: Foxtalk Expr..."]

----- BodyWriter -----
data WriterState = WriterState { bodyIndentLevel :: Int, bodyLines :: [(Int, String)] }
type BodyWriter = StateT WriterState (Either String)

bodyWrite :: String -> BodyWriter ()
bodyWrite l =
  do
    ilevel <- gets bodyIndentLevel
    modify (\s -> WriterState (bodyIndentLevel s) (bodyLines s ++ [(ilevel, l)]))

indent :: BodyWriter ()
indent = modify (\s -> WriterState (bodyIndentLevel s + 2) (bodyLines s))

outdent :: BodyWriter ()
outdent = modify (\s -> WriterState (max (bodyIndentLevel s - 2) 0) (bodyLines s))

writeCExpr :: CExpr -> BodyWriter ()
writeCExpr c = bodyWrite c

----- GENPROG -----
genWhenBody :: Int -> (QueryExpr String) -> [HandlerBodyLine String] -> BodyWriter ()
genWhenBody n q bod =
  do
    let qr  = "queryResults" ++ show n
    let res = "res" ++ show n
    bodyWrite $ "void handle(const std::vector<Tuple> &" ++ qr ++ ") override"
    bodyWrite   "{"
    indent
    bodyWrite $ "for(auto& " ++ res ++ " : " ++ qr ++ ") {"
    indent
    mapM_ writeCExpr $ queryExprToLookup res $ queryExprToPosition q
    bodyWrite ""
    mapM_ bodyWrite $ queryExprToLookupGuard q
    bodyWrite ""

    lns <- lift (concat <$> traverse handlerBodyToCExpr bod)
    mapM_ bodyWrite lns

    outdent
    bodyWrite "}"
    outdent
    bodyWrite "}"

genHandleBody :: Int -> FoxtalkExpr String -> BodyWriter ()
genHandleBody _ (EClaim _) = throwError "Cannot generate a handler body from a claim"
genHandleBody n (EWhen q bod) = genWhenBody n q bod
genHandleBody _ (EForAll _ _ _) = undefined

genQueryBody :: FoxtalkExpr String -> BodyWriter ()
genQueryBody expr =
  do
    bodyWrite "void init() override"
    bodyWrite "{"
    indent
    claims <- lift $ exprToHandlerQuery expr
    mapM_ bodyWrite claims
    outdent
    bodyWrite "}"

genProg :: String -> [FoxtalkExpr String] -> BodyWriter ()
genProg handlerName exprs =
  do
    bodyWrite "#include <foxtalk_handler.hpp>"
    bodyWrite ""
    bodyWrite $ "class " ++ handlerName ++ " : public Handler"
    bodyWrite "{"
    indent
    bodyWrite "protected:"
    genQueryBody (exprs !! 0)
    genHandleBody 0 (exprs !! 0)
    outdent
    bodyWrite "}"

runFoxtalkCodegen :: BodyWriter () -> Either String String
runFoxtalkCodegen w = unlines . map indentedLine . bodyLines <$> execStateT w (WriterState 0 [])
  where
    indentedLine :: (Int, String) -> String
    indentedLine (0, s) = s
    indentedLine (i, s) = ' ' : indentedLine (i-1, s)
