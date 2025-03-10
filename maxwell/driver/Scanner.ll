/* Copyright (c) 2013-2014 Fabian Schuiki */

%{
#include "maxwell/ast/nodes/ast.hpp"
#include "maxwell/location.hpp"
#include <sstream>
#include <string>
#include <vector>

using namespace ast;
typedef std::vector<NodePtr> Nodes;

#include "maxwell/driver/Scanner.hpp"

#define storeToken yylval->string = new std::string(yytext, yyleng)

/* Add local typedefs for the parser's tokens for our convenience. */
typedef driver::Parser::token token;
typedef driver::Parser::token_type token_type;

/* Make yyterminate return a special token instead of 0. */
#define yyterminate() return token::END;

/* Disable unistd.h which is not available under Windows. */
#define YY_NO_UNISTD_H

/* Flex uses the "register" keyword all over the place, which is deprecated. */
#define register
%}

/* Enable the C++ magic. */
%option c++

/* Change the name of the lexer class to "DriverFlexLexer" */
%option prefix="Driver"

/* The manual says "somewhat more optimized", whatever that means. */
%option batch

/* Produce some debug output. Disable this for the release version. */
%option debug

/* Advance the location tracker during lexing. */
%{
// #define YY_USER_ACTION yylloc->columns(yyleng); streamOffset += yyleng;
#define YY_USER_ACTION *yylloc = maxwell::SourceRange( \
    yylloc->getSourceId(), \
    yylloc->getOffset(), \
    yylloc->getLength() + yyleng);
%}

%x IN_COMMENT

additive_op         [+\-]
multiplicative_op   [*%/]
relational_op       [<>]=?
equality_op         [!=]=
and_op              "&&"
or_op               "||"
symbol              {additive_op}|{multiplicative_op}|{relational_op}|{equality_op}|{and_op}|{or_op}|[!\.:~&''^]

%% /*** Token Regular Expressions ***/

 /* Whenever yylex() is invoked, we step the location tracker forward. */
%{
    // reset location
    // yylloc->step();
    *yylloc = maxwell::SourceRange(yylloc->getEnd(), yylloc->getEnd());
%}

 /* Ignore whitespaces. */
[ \t\r]+ {
    // yylloc->step();
    *yylloc = maxwell::SourceRange(yylloc->getEnd(), yylloc->getEnd());
}

 /* Ignore newlines. */
<INITIAL,IN_COMMENT>\n {
    // yylloc->lines(yyleng);
    // yylloc->step();
    *yylloc = maxwell::SourceRange(yylloc->getEnd(), yylloc->getEnd());
}

 /* Eat comments. */
"//"[^\n]*  ; /* we might want to keep track of the comments, for automated documentation */
<INITIAL>{
    "/*"      BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
    "*/"      BEGIN(INITIAL);
    [^*\n]+   // eat comment in chunks
    "*"       // eat the lone star
}

 /* Eat strings. */
"\""(\\.|[^\\"\""])*"\""  storeToken; return token::STRING_LITERAL;

 /* Keywords */
"func"       return token::FUNC;
"var"        return token::VAR;
"type"       return token::TYPE;
"unary"      return token::UNARY;
"value"      return token::VALUE;
"object"     return token::OBJECT;
"interface"  return token::INTERFACE;
"native"     return token::NATIVE;
"range"      return token::RANGE;
"if"         return token::IF;
"else"       return token::ELSE;
"for"        return token::FOR;
"incase"     return token::INCASE;
"otherwise"  return token::OTHERWISE;
"cast"       return token::CAST;
"nil"        return token::NIL;

 /* Constants */
[a-zA-Z_][a-zA-Z0-9_!?=]*  storeToken; return token::IDENTIFIER;
"-"?[0-9]+("."[0-9]*)?     storeToken; return token::NUMBER;

 /* Punctuation */
"("  return token::LPAREN;
")"  return token::RPAREN;
"{"  return token::LBRACE;
"}"  return token::RBRACE;
"["  return token::LBRACK;
"]"  return token::RBRACK;

"."  return token::DOT;
","  return token::COMMA;
":"  return token::COLON;
";"  return token::SEMICOLON;

"->" return token::RIGHTARROW;
"|"  return token::PIPE;
"="  return token::ASSIGN;
":=" return token::DEFASSIGN;
"#"  return token::HASHTAG;
"&"  return token::AMPERSAND;

({additive_op}{symbol}*|{symbol}*{additive_op})             storeToken; return token::ADDITIVE_OPERATOR;
({multiplicative_op}{symbol}*|{symbol}*{multiplicative_op}) storeToken; return token::MULTIPLICATIVE_OPERATOR;
({relational_op}{symbol}*|{symbol}*{relational_op})         storeToken; return token::RELATIONAL_OPERATOR;
({equality_op}{symbol}*|{symbol}*{equality_op})             storeToken; return token::EQUALITY_OPERATOR;
({and_op}{symbol}*|{symbol}*{and_op})                       storeToken; return token::AND_OPERATOR;
({or_op}{symbol}*|{symbol}*{or_op})                         storeToken; return token::OR_OPERATOR;
{symbol}*                                                   storeToken; return token::OPERATOR;

 /* All other characters are rejected. */
. {
    std::stringstream s;
    s << "Unknown token '" << yytext[0] << "'";
    throw std::runtime_error(s.str());
}


%% /*** Additional Code ***/

/* Wrapper class around the lexer created by Flex which makes the interface look similar to the parser. */
namespace driver {

Scanner::Scanner(std::istream* in, std::ostream* out) : DriverFlexLexer(in, out)
{
}

Scanner::~Scanner()
{
}

}

#ifdef yylex
#undef yylex
#endif

/**
 * This implementation of DriverFlexLexer::yylex() is required to fill the
 * vtable of the class DriverFlexLexer. We define the scanner's main yylex
 * function via YY_DECL to reside in the Scanner class instead.
 */
int DriverFlexLexer::yylex()
{
    std::cerr << "in DriverFlexLexer::yylex() !" << std::endl;
    return 0;
}

/**
 * When the scanner receives an end-of-file indication from YY_INPUT, it then
 * checks the yywrap() function. If yywrap() returns false (zero), then it is
 * assumed that the function has gone ahead and set up `yyin' to point to
 * another input file, and scanning continues. If it returns true (non-zero),
 * then the scanner terminates, returning 0 to its caller.
 */
int DriverFlexLexer::yywrap()
{
    return 1;
}
