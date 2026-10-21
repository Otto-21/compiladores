#pragma once

#include <stdio.h>

extern FILE *source; // TBD at main.c
extern FILE *output; // TBD at main.c

extern int line;
extern char lexeme[]; //stores the token string content
extern int lookahead;
extern int parser_status;
typedef enum {
	PARSER_OK = 0,
	ERR_TOKEN_MISMATCH,
	ERR_INVALID_START,
	ERR_EXPECTED_SEPARATOR,
	ERR_DIVISION_BY_ZERO,
	ERR_UNDECLARED_VARIABLE,
	ERR_SYMBOL_TABLE_OVERFLOW,
	ERR_STACK_OVERFLOW,
	ERR_STACK_UNDERFLOW
} ParserError;

extern int expr(void);
extern int match(int required);
extern void parser_error(ParserError error);
extern int getToken(FILE *source); //defined at lexer.c