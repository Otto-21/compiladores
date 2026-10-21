//<vmachine.c::

#include<symtab.h>
#include<vmachine.h>
#include<parser.h>
#include<stdio.h>

// virtual machine definition
// accumulator:
double acc=0;
double stack[MAXSTACKSIZE];
int sp = -1;

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

int recall(const char *variable, double *value){
	int pos = symtab_lookup(variable); //procura a variavel, se nao existir, pos = -1
	if(pos < 0){
		parser_error(ERR_UNDECLARED_VARIABLE);
		return 0;
	}
	*value = vmem[pos];
	return 1;
}

int push(void){
	if(sp < MAXSTACKSIZE - 1){
        sp++;
        stack[sp] = acc;
		return 1;
    }
	parser_error(ERR_STACK_OVERFLOW);
	return 0;
}

int pop(double *value){
	if(sp >= 0){
        *value = stack[sp];
        sp--;
        return 1;
    }
	parser_error(ERR_STACK_UNDERFLOW);
	return 0;
}