//<symtab.c::

#include<stdio.h>
#include<string.h>
#include<lexer.h>
#include<symtab.h>

double vmem[SYMTABSIZE];
char symtab[SYMTABSIZE][MAXSTRLEN + 1];
int symtab_nextentry = 0;

int symtab_lookup(const char *varname){
    int i;
	for(i=0; i < symtab_nextentry; i++){
		if(strcmp(symtab[i], varname) == 0){
			return i;
		}
	}
	return -1; //ajustar constantes para erros especificos - varname not found
}

int symtab_append(const char *varname){
    if(symtab_nextentry < SYMTABSIZE){
        int pos = symtab_nextentry;
        strcpy(symtab[symtab_nextentry++], varname);
        return pos;
    }
    return -1; //ajustar constantes para erros especificos - symtab overflow
}