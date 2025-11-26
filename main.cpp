#include <iostream>
#include "ast.h"

int yyparse();
extern ASTNode *root;

int main() {
    if (yyparse() == 0) {
        std::cout << "Parse OK\n";
        std::cout << "--- Program output ---" << std::endl;
        eval_program(root);
        ast_print_dot(root, "ast.dot");
        std::cout << "--- AST written to ast.dot ---" << std::endl;
        free_ast(root);
    } else {
        std::cout << "Parse FAILED\n";
    }
    return 0;
}
