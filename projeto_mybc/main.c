//<main.c::

#include <stdio.h>
#include <ctype.h>
#include <tokens.h>
#include <main.h>

FILE *source, *output;

// Define a entrada pelo terminal e a saida pela tela.
// Le o primeiro token, executa o parser e retorna seu estado de erro.
int main(void){
	source= stdin;
	output= stdout;

	lookahead = getToken(stdin);

	mybc();
	
	return parser_status;
}

