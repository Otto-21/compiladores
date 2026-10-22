//<lexer.c::

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <lexer.h>
#include <tokens.h>
//#include <main.h>

int line = 1;
char lexeme[MAXSTRLEN + 1];

// Informa a linha e o motivo de um erro encontrado pelo lexer.
static void lexer_error(const char *message)
{
	fprintf(stderr, "lexer error at line %d: %s\n", line, message);
	exit(-3);
}

// Avanca pelos espacos em branco antes de ler um token.
// Atualiza o numero da linha e devolve o primeiro caractere util.
void skipspaces(FILE *tape)
{
	int c;
	while (isspace(c = getc(tape))) {
		if (c == '\n'){
			line++;
			break;
		}
	}
	if (c != EOF)
		ungetc(c, tape);
}

// Descarta comentarios no formato { ... } e os espacos entre eles.
// Conta as quebras de linha e avisa se chegar ao fim sem fechar um comentario.
void skipcomments(FILE *tape)
{
	int head;

	// Volta aqui quando encontra mais de um comentario seguido.
	_skpspc:
	skipspaces(tape);
	
	if( (head = getc(tape)) == '{' ){
		while( (head = getc(tape)) != '}' ){

			if(head == '\\'){
				//se encontrar um '\', ignora o caracter que vier posteriormente
				if( (head = getc(tape)) == '{' || head == '}')
					head = getc(tape);
			}

			if(head == '\n') line++;
		
			if(head == EOF){
					lexer_error("unclosed comment");
				}
			}
			goto _skpspc;
	}
	if(head != EOF) ungetc(head, tape);
}

// Verifica se os proximos caracteres formam o operador ':='.
// Se formar, grava o lexema e retorna ASGN; caso contrario, restaura a leitura.
int isASGN(FILE *tape){
	int first = getc(tape);
	if (first == ':') {
			int second = getc(tape);
			if (second == '=') {
				lexeme[0] = ':';
				lexeme[1] = '=';
            lexeme[2] = 0;
            return ASGN;
        }
        if (second != EOF)
				ungetc(second, tape);
    }
    if (first != EOF)
			ungetc(first, tape);
    return 0;
}

// Le um numero, incluindo parte decimal e expoente (como 1.5e2).
// Grava os caracteres em lexeme e valida o formato antes de retornar NUM.
int isNUM(FILE *tape){
	int c = getc(tape);
	int i = 0;
	char *end;

	// Permite numeros que comecam pelo ponto, como .5.
	if (c == '.') {
			int next = getc(tape);
			if (!isdigit(next)) {
				if (next != EOF)
					ungetc(next, tape);
				ungetc(c, tape);
				return 0;
			}
			lexeme[i++] = (char)c;
			c = next;
	} else if (!isdigit(c)) {
			if (c != EOF)
				ungetc(c, tape);
			lexeme[0] = 0;
			return 0;
	}

	while (isdigit(c) || c == '.' || c == 'e' || c == 'E' ||
			   ((c == '+' || c == '-') && i > 0 &&
			    (lexeme[i - 1] == 'e' || lexeme[i - 1] == 'E'))) {
			if (i >= MAXSTRLEN)
				lexer_error("numeric literal too long");
			lexeme[i++] = (char)c;
			c = getc(tape);
	}
	lexeme[i] = 0;
	if (c != EOF)
			ungetc(c, tape);

	// Confirma que toda a sequencia lida forma um numero valido.
	(void)strtod(lexeme, &end);
	if (*end != 0)
			lexer_error("malformed numeric literal");
	return NUM;
}

// Reconhece nomes iniciados por letra e seguidos de letras ou digitos.
// Armazena o nome em lexeme, respeitando o limite de tamanho.
int isID(FILE *tape){
	/*
	 ID = [A-Za-z][A-Za-z0-9]
	*/
	int c = getc(tape);
	if (isalpha(c)) {
		lexeme[0] = (char)c;
		int i=1;
		while (isalnum(c = getc(tape))) {
			if (i >= MAXSTRLEN)
				lexer_error("identifier too long");
			lexeme[i++] = (char)c;
		}
		if (c != EOF)
			ungetc(c, tape);
		lexeme[i] = 0; //toda string termina com 0
		return ID;
	}
	if (c != EOF)
		ungetc(c, tape);
	return 0;
}

// Ponto de entrada do analisador lexico: pula comentarios e tenta
// reconhecer ID, NUM ou ASGN. Outros caracteres sao retornados diretamente.
int getToken(FILE *source){
	int token;

	skipcomments(source);

	if((token = isID(source))) return token;
	if((token = isNUM(source))) return token;
	if((token = isASGN(source))) return token;

	token = getc(source);
	return token;
}
