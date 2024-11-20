// use nom::branch::alt;
// use nom::bytes::complete::*;
// use nom::character::complete::*;
// use nom::error::{ErrorKind, ParseError};
// use nom::multi::many1;
// use nom::Err::Error;
// use nom::IResult;
// 
// #[derive(Debug, PartialEq, Eq)]
// enum QueryToken {
//     TSymbolLit(String),
//     TVarIntro(String),
//     TVarBinding(String),
//     TVarIntroLit(String, String),
//     TLParen,
//     TRParen,
//     TOr,
//     TAnd,
// }
// 
// 
// #[derive(Debug, PartialEq)]
// pub enum QueryTokenError<I> {
//     InvalidUnwrap,
//     Nom(I, ErrorKind),
// }
// pub type QResult<I, T> = IResult<I, T,QueryTokenError<I>>;
// impl<I> ParseError<I> for QueryTokenError<I> {
//     fn from_error_kind(input: I, kind: ErrorKind) -> Self {
//         QueryTokenError::Nom(input, kind)
//     }
// 
//     fn append(_: I, _: ErrorKind, other: Self) -> Self {
//         other
//     }
// }
// impl QueryToken {
//     // Maybe better to implement Try so that you can just do like ?, 
//     // but that's an unstable feature and this is fine for now.
//     fn unwrap(self, i: &str) -> QResult<&str, String> {
//         match self {
//             TSymbolLit(s) => Ok((i, s.clone())),
//             TVarIntro(s) => Ok((i, s.clone())),
//             TVarBinding(s) => Ok((i, s.clone())),
//             TVarIntroLit(_, _) =>  Err(Error(QueryTokenError::InvalidUnwrap)),
//             TLParen => Err(Error(QueryTokenError::InvalidUnwrap)),
//             TRParen => Err(Error(QueryTokenError::InvalidUnwrap)),
//             TOr => Err(Error(QueryTokenError::InvalidUnwrap)),
//             TAnd => Err(Error(QueryTokenError::InvalidUnwrap)),
//         }
//     }
// }
// 
// 
// use QueryToken::*;
// 
// fn ident(i: &str) -> QResult<&str, QueryToken> {
//     let (i, s) = alphanumeric1(i)?;
//     Ok((i, TSymbolLit(s.to_string())))
// }
// 
// fn l_paren(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag("(")(i)?;
//     Ok((i, TLParen))
// }
// fn r_paren(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag(")")(i)?;
//     Ok((i, TRParen))
// }
// 
// fn or_word(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag("or")(i)?;
//     Ok((i, TOr))
// }
// fn and_word(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag("and")(i)?;
//     Ok((i, TAnd))
// }
// fn var_binding(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag("(")(i)?;
//     let (i, var_name_qt) = ident(i)?;
//     let (i, var_name) = var_name_qt.unwrap(i)?;
//     let (i, _) = tag(")")(i)?;
//     Ok((i, TVarBinding(var_name.to_string())))
// }
// 
// fn var_intro(i: &str) -> QResult<&str, QueryToken> {
//     let (i, _) = tag("/")(i)?;
//     let (i, var_name_qt) = ident(i)?;
//     let (i, var_name) = var_name_qt.unwrap(i)?;
//     let (i, _) = tag("/")(i)?;
//     match tag::<&str, &str, QueryTokenError<&str>>("@")(i) {
//         Ok((i, _)) => {
//             let (i, str_lit_qt) = ident(i)?;
//             let (i, str_lit) = str_lit_qt.unwrap(i)?;
//             Ok((i, TVarIntroLit(var_name, str_lit.to_string())))
//         }
//         _ => Ok((i, TVarIntro(var_name.to_string()))),
//     
//     }
// }
// 
// fn token(input: &str) -> QResult<&str, QueryToken> {
//     alt((
//         and_word,
//         or_word,
//         var_binding,
//         l_paren,
//         r_paren,
//         var_intro,
//         ident,
//     ))(input)
// }
// fn token_and_whitespace(input: &str) -> QResult<&str, QueryToken> {
//     let (input, res) = token(input)?;
//     let (input, _) = multispace0(input)?;
//     Ok((input, res))
// }
// fn tokens_parser(input: &str) -> QResult<&str, Vec<QueryToken>> {
//     many1(token_and_whitespace)(input)
// }
// fn tokens(input: &str) -> Option<Vec<QueryToken>> {
//     if let Ok(("", res)) = tokens_parser(input) {
//         Some(res)
//     } else {
//         None
//     }
// }
// 
// #[cfg(test)]
// mod tests {
//     use super::*;
// 
//     #[test]
//     fn test_ident_1() {
//         let input = "a";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TSymbolLit("a".to_string())]);
//     }
//     #[test]
//     fn test_ident_2() {
//         let input = "fox";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TSymbolLit("fox".to_string())]);
//     }
//     #[test]
//     fn test_ident_3() {
//         let input = "foxorlexi";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TSymbolLit("foxorlexi".to_string())]);
//     }
//     #[test]
//     fn test_var_intro_1() {
//         let input = "/lexi/";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TVarIntro("lexi".to_string())]);
//     }
//     #[test]
//     fn test_var_intro_2() {
//         let input = "/test with spaces/";
//         let result = tokens(input);
//         assert!(result.is_none());
//     }
//     #[test]
//     fn test_var_binding_1() {
//         let input = "(lexi)";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TVarBinding("lexi".to_string())]);
//     }
//     #[test]
//     fn test_var_binding_2() {
//         let input = "(test with spaces)";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TLParen, TSymbolLit("test".to_string()), TSymbolLit("with".to_string()), TSymbolLit("spaces".to_string()), TRParen]);
//     }
//     #[test]
//     fn test_bount_lit_1() {
//         let input = "/shape/@circle";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         assert_eq!(output, [TVarIntroLit("shape".to_string(), "circle".to_string())]);
//     }
//     #[test]
//     fn test_tokens_1() {
//         let input = "/shape/@rectangle with x /x/ or /shape/@circle with r /r/";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
// 
//         let expected = vec![
//             TVarIntroLit("shape".to_string(), "rectangle".to_string()),
//             TSymbolLit("with".to_string()),
//             TSymbolLit("x".to_string()),
//             TVarIntro("x".to_string()),
//             TOr,
//             TVarIntroLit("shape".to_string(), "circle".to_string()),
//             TSymbolLit("with".to_string()),
//             TSymbolLit("r".to_string()),
//             TVarIntro("r".to_string())
//         ];
// 
//         assert_eq!(output, expected);
//     }
//     #[test]
//     fn test_tokens_2() {
//         let input = "((you) is a rectangle with x /x/) and ((you) has color /c/)";
//         let result = tokens(input);
//         assert!(result.is_some());
//         let output = result.unwrap();
//         let expected = vec![
//             TLParen,
//             TVarBinding("you".to_string()),
//             TSymbolLit("is".to_string()),
//             TSymbolLit("a".to_string()),
//             TSymbolLit("rectangle".to_string()),
//             TSymbolLit("with".to_string()),
//             TSymbolLit("x".to_string()),
//             TVarIntro("x".to_string()),
//             TRParen,
//             TAnd,
//             TLParen,
//             TVarBinding("you".to_string()),
//             TSymbolLit("has".to_string()),
//             TSymbolLit("color".to_string()),
//             TVarIntro("c".to_string()),
//             TRParen
//         ];
//         assert_eq!(output, expected);
//     }
//     
// }