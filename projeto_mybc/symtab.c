//<symtab.c::

#include<stdio.h>
#include<string.h>
#include<lexer.h>
#include<symtab.h>

// symtab guarda os nomes; vmem guarda o valor na mesma posicao.
// symtab_nextentry indica a proxima posicao livre.
double vmem[SYMTABSIZE];
char symtab[SYMTABSIZE][MAXSTRLEN + 1];
int symtab_nextentry = 0;

// Percorre as entradas cadastradas procurando o nome da variavel.
// Retorna sua posicao na tabela ou -1 quando nao encontra.
int symtab_lookup(const char *varname){
    int i;
	for(i=0; i < symtab_nextentry; i++){
		if(strcmp(symtab[i], varname) == 0){
			return i;
		}
	}
	return -1;
}

// Registra um novo nome na proxima posicao livre da tabela.
// Retorna a posicao usada ou -1 quando a tabela esta cheia.
int symtab_append(const char *varname){
    if(symtab_nextentry < SYMTABSIZE){
        int pos = symtab_nextentry;
        strcpy(symtab[symtab_nextentry++], varname);
        return pos;
    }
    return -1;
}
