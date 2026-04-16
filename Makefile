CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic
COVERAGE_FLAGS := --coverage -O0

all:
	bison -d parser.y
	flex lexer.l
	$(CXX) $(CXXFLAGS) -o mini_c main.cpp ast.cpp parser.tab.c lex.yy.c

test:
	$(CXX) $(CXXFLAGS) -Itests -o ast_tests tests/ast_tests.cpp ast.cpp
	./ast_tests

coverage:
	$(CXX) $(CXXFLAGS) $(COVERAGE_FLAGS) -Itests -o ast_tests_cov tests/ast_tests.cpp ast.cpp
	./ast_tests_cov
	gcov -b ast_tests_cov-ast.gcno

clean:
	rm -f mini_c parser.tab.* lex.yy.c ast.dot ast_tests ast_tests_cov *.gcda *.gcno *.gcov tests/tmp_ast.dot
