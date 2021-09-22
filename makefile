BIN = ./bin/

$(shell mkdir -p $(BIN))

assembler: assembler.o assembler.o asmencoder.o asmfile.o buffer.o callbacks.o command.o directive.o memohelper.o mytypes.o parsing.o statement.o strhelper.o
	gcc -g -ansi -Wall -pedantic ${BIN}assembler.o ${BIN}asmencoder.o ${BIN}asmfile.o ${BIN}buffer.o ${BIN}callbacks.o ${BIN}command.o ${BIN}directive.o ${BIN}memohelper.o ${BIN}mytypes.o ${BIN}parsing.o ${BIN}statement.o ${BIN}strhelper.o -o ${BIN}assembler

assembler.o: main.c asmfile.h asmencoder.o
	gcc -c -g -ansi -Wall -pedantic main.c -o ${BIN}assembler.o

asmencoder.o: asmencoder.c asmencoder.h mytypes.o strhelper.o asmfile.o command.o directive.o
	gcc -c -g -ansi -Wall -pedantic asmencoder.c -o ${BIN}asmencoder.o

asmfile.o: asmfile.c asmfile.h mytypes.o buffer.o statement.o
	gcc -c -g -ansi -Wall -pedantic asmfile.c -o ${BIN}asmfile.o

buffer.o: buffer.c buffer.h mytypes.o
	gcc -c -g -ansi -Wall -pedantic buffer.c -o ${BIN}buffer.o

callbacks.o: callbacks.c callbacks.h parsing.o asmencoder.o
	gcc -c -g -ansi -Wall -pedantic callbacks.c -o ${BIN}callbacks.o

command.o: command.c command.h strhelper.o mytypes.o
	gcc -c -g -ansi -Wall -pedantic command.c -o ${BIN}command.o

directive.o: directive.c directive.h strhelper.o mytypes.o
	gcc -c -g -ansi -Wall -pedantic directive.c -o ${BIN}directive.o

memohelper.o: memohelper.c memohelper.h
	gcc -c -g -ansi -Wall -pedantic memohelper.c -o ${BIN}memohelper.o

mytypes.o: mytypes.c mytypes.h
	gcc -c -g -ansi -Wall -pedantic mytypes.c -o ${BIN}mytypes.o

parsing.o: parsing.c parsing.h mytypes.h memohelper.o command.o directive.o
	gcc -c -g -ansi -Wall -pedantic parsing.c -o ${BIN}parsing.o

statement.o: statement.c statement.h mytypes.o buffer.o strhelper.o
	gcc -c -g -ansi -Wall -pedantic statement.c -o ${BIN}statement.o

strhelper.o: strhelper.c strhelper.h mytypes.o memohelper.o
	gcc -c -g -ansi -Wall -pedantic strhelper.c -o ${BIN}strhelper.o
