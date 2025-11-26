all:
	bison -d parser.y
	flex lexer.l
	g++ -std=c++20 -o mini_c main.cpp ast.cpp parser.tab.c lex.yy.c

clean:
	rm -f mini_c parser.tab.* lex.yy.c ast.dot
