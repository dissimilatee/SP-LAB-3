#ifndef AST_H
#define AST_H

#include <string>
#include <unordered_map>
#include <iostream>

enum NodeType {
    N_NUM,
    N_VAR,
    N_BINOP,
    N_ASSIGN,
    N_PRINT,
    N_IF,
    N_WHILE,
    N_SEQ
};

struct ASTNode {
    NodeType type;
    int value = 0;
    std::string name;
    char op = 0;

    ASTNode *left = nullptr;
    ASTNode *right = nullptr;
    ASTNode *cond = nullptr;
    ASTNode *then_branch = nullptr;
    ASTNode *else_branch = nullptr;
    ASTNode *next = nullptr;
};

ASTNode *new_num(int v);
ASTNode *new_var(std::string v);
ASTNode *new_binop(char op, ASTNode *l, ASTNode *r);
ASTNode *new_assign(std::string var, ASTNode *expr);
ASTNode *new_print(ASTNode *expr);
ASTNode *new_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b);
ASTNode *new_while(ASTNode *cond, ASTNode *body);
ASTNode *new_seq(ASTNode *a, ASTNode *b);

void eval_program(ASTNode *n);
void ast_print_dot(ASTNode *n, const std::string &fname);
void free_ast(ASTNode *n);

#endif
