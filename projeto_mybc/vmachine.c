//<vmachine.c::

#include<symtab.h>
#include<vmachine.h>
#include<parser.h>
#include<stdio.h>

// virtual machine definition
// accumulator:
// acc guarda o resultado atual das expressoes.
// stack guarda valores intermediarios e sp indica o topo da pilha.
double acc=0;
double stack[MAXSTACKSIZE];
int sp = -1;

// Associa o valor de acc ao nome de uma variavel.
// Se o nome nao existe, cria a entrada; falha caso a tabela esteja cheia.
int store(const char *variable){
	int pos = symtab_lookup(variable);
	if(pos < 0){
		pos = symtab_append(variable);
	}
	if (pos < 0) {
		parser_error(ERR_SYMBOL_TABLE_OVERFLOW);
		return 0;
	}
	vmem[pos] = acc;
	return 1;
}

// Consulta a tabela de simbolos para recuperar o valor de uma variavel.
// Se ela ainda nao existe, informa o erro de variavel nao declarada.
int recall(const char *variable, double *value){
	int pos = symtab_lookup(variable); //procura a variavel, se nao existir, pos = -1
	if(pos < 0){
		parser_error(ERR_UNDECLARED_VARIABLE);
		return 0;
	}
	*value = vmem[pos];
	return 1;
}

// Guarda o valor atual de acc na pilha para uma operacao posterior.
// Impede a insercao se a pilha ja atingiu o limite.
int push(void){
	if(sp < MAXSTACKSIZE - 1){
        sp++;
        stack[sp] = acc;
		return 1;
    }
	parser_error(ERR_STACK_OVERFLOW);
	return 0;
}

// Recupera pelo parametro value o ultimo valor salvo na pilha.
// Atualiza sp e informa erro se nao houver valor para retirar.
int pop(double *value){
	if(sp >= 0){
        *value = stack[sp];
        sp--;
        return 1;
    }
	parser_error(ERR_STACK_UNDERFLOW);
	return 0;
}