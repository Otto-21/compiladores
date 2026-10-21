//<parser.c::

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<tokens.h>
#include<symtab.h>
#include<lexer.h>
#include<vmachine.h>
#include<parser.h>

// lookahead is the compiler's eye whice is traditionally defined here
int lookahead;
int parser_status;
static int expression_error;
static int mismatch_received;
static int mismatch_expected;

static const char *const error_messages[] = {
	"no error",
	"token mismatch",
	"invalid token at start of expression",
	"expected expression separator",
	"division by zero",
	"use of undeclared variable",
	"symbol table overflow",
	"operand stack overflow",
	"operand stack underflow"
};

static void format_token(int token, char *buffer, size_t buffer_size)
{
	const char *name = NULL;

	switch (token) {
	case EOF:
		name = "EOF";
		break;
	case ID:
		name = "ID";
		break;
	case NUM:
		name = "NUM";
		break;
	case ASGN:
		name = "ASGN";
		break;
	case '\n':
		name = "newline";
		break;
	case '\t':
		name = "tab";
		break;
	case ' ':
		name = "space";
		break;
	default:
		break;
	}

	if (name != NULL) {
		snprintf(buffer, buffer_size, "%s", name);
	} else if (token >= 0 && isprint((unsigned char)token)) {
		snprintf(buffer, buffer_size, "'%c'", token);
	} else {
		snprintf(buffer, buffer_size, "token %d", token);
	}
}

/*
	LL(1) recursive parser requires an LL(1) grammar.
	(LL = Left-to-right scan, leftmost derivation)

	LL(1) for expressions

	E -> ['+' '-'] T R
	T -> F Q
	R -> '+' T R | '-' T R | <empty>
	Q -> '*' F Q | '/' F Q | <empty>
	F -> ID | DEC | '(' E ')'
*/

int expr(void){
	int sigflag=0, otimesflg=0, oplusflg=0;
	char name[MAXSTRLEN+1];
	double left;

	//check prefixing signal
	if(lookahead == '+' || lookahead == '-'){
		if(lookahead == '-'){
			sigflag=1; //signal seen
		}
		if (!match(lookahead))
			return 0;
	}

	_T:

	_F:

	switch(lookahead){
	case ID:
		strcpy(name, lexeme);
		if (!match(ID))
			return 0;
		if(lookahead == ASGN){
			// ID := E -> L-value 
			if (!match(ASGN) || !expr() || !store(name))
				return 0;
		}
		else{
			// R-value
			if (!recall(name, &acc))
				return 0;
		}
		break;
	case NUM:
		acc = atof(lexeme); //ação semantica
		if (!match(NUM))			//ação gramatical
			return 0;
		break;
	default:
		if (!match('(') || !expr() || !match(')'))
			return 0;
	}

	if(otimesflg){
		if(otimesflg == '*'){
			if (!pop(&left))
				return 0;
			acc = left * acc;
		}
		else{
			if (acc == 0.0)
			{
				parser_error(ERR_DIVISION_BY_ZERO);
				return 0;
			}
			if (!pop(&left))
				return 0;
			acc = left / acc;
		}
		if (expression_error)
			return 0;
		otimesflg = 0;
	}

	if(lookahead == '*' || lookahead == '/'){
		if (!push())
			return 0;
		otimesflg = lookahead;
		if (!match(lookahead))
			return 0;
		goto _F;
	}

	if(sigflag){
		acc = -acc;
		sigflag = 0;
	}

	if(oplusflg){
		if (!pop(&left))
			return 0;
		if(oplusflg == '+'){
			acc = left + acc;
		}
		else{
			acc = left - acc;
		}
		oplusflg = 0;
	}

	if(lookahead == '+' || lookahead == '-'){
		if (!push())
			return 0;
		oplusflg = lookahead;
		if (!match(lookahead))
			return 0;
		goto _T;
	}
	return 1;
}

int match(int required){
	/*
		O que match() faz? Verifica se o token atual gerado pelo lexer é
		exatamente o que o compilador espera na gramática da linguagem.
 		Comportamento: Se o token for o esperado, ela consome o token e
		avança para o próximo. Se não for, dispara um erro de sintaxe.
	*/

	if(lookahead == required){
		lookahead = getToken(source);
		return 1;
	}
	mismatch_received = lookahead;
	mismatch_expected = required;
	parser_error(ERR_TOKEN_MISMATCH);
	return 0;
}

void parser_error(ParserError error)
{
	int error_line = line;
	parser_status = 1;
	expression_error = 1;
	if (error < 0 || error >= (int)(sizeof(error_messages) /
			sizeof(error_messages[0])))
		error = ERR_TOKEN_MISMATCH;
	if (error == ERR_TOKEN_MISMATCH && mismatch_received == '\n' &&
	    error_line > 1)
		error_line--;
	fprintf(stderr, "error at line %d: %s", error_line, error_messages[error]);
	if (error == ERR_TOKEN_MISMATCH) {
		char received[32];
		char expected[32];
		format_token(mismatch_received, received, sizeof(received));
		format_token(mismatch_expected, expected, sizeof(expected));
		fprintf(stderr, ": received %s, expected %s", received, expected);
	}
	fprintf(stderr, "\n");
}

int isFIRST(void){
	switch(lookahead){
		case '+':
		case '-':
		case ID:
		case NUM:
		case '(':
			return lookahead;
	}
	return 0;
}

void mybc(){
	while(lookahead != EOF){
		int valid = 1;
		expression_error = 0;

		if(isFIRST()){
			valid = expr();
			if (valid && lookahead != '\n' && lookahead != ';' &&
			    lookahead != EOF) {
				parser_error(ERR_EXPECTED_SEPARATOR);
				valid = 0;
			}
			if (valid) {
				fprintf(output, "%lg\n", acc);
				fflush(output);
			}
		} else {
			parser_error(ERR_INVALID_START);
			valid = 0;
		}

		if (!valid) {
			while (lookahead != '\n' && lookahead != ';' &&
				   lookahead != EOF)
				lookahead = getToken(source);
			sp = -1;
			if (lookahead == EOF)
				break;
		}
		if(lookahead == '\n' || lookahead == ';'){
			match(lookahead);
		}
	}
}