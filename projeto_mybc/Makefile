CFLAGS = -I. -g

EXEC = mybc

$(EXEC): main.o lexer.o parser.o symtab.o vmachine.o
	$(CC) -o $@ $^

# depolution targets:

clean:
	$(RM) *.o

mostlyclean: clean
	$(RM) *~ 