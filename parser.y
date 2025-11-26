%{
#include <iostream>
#include <string>
#include "ast.h"

int yylex();
void yyerror(const char *s);

ASTNode *root = nullptr;
%}

%union {
    int ival;
    std::string *sval;
    ASTNode *node;
}

%token <ival> T_NUM
%token <sval> T_ID
%token T_IF T_ELSE T_WHILE T_PRINT
%token T_EQ T_NE T_LE T_GE

%type <node> program stmt stmt_list expr cond

%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '<' '>' T_LE T_GE T_EQ T_NE
%nonassoc LOWER_THAN_ELSE
%nonassoc T_ELSE

%%

program
    : stmt_list               { root = $1; $$ = root; }
    ;

stmt_list
    : stmt                    { $$ = $1; }
    | stmt_list stmt          { $$ = new_seq($1, $2); }
    ;

stmt
    : T_ID '=' expr ';'       { $$ = new_assign(*$1, $3); delete $1; }
    | T_PRINT '(' expr ')' ';'{ $$ = new_print($3); }
    | T_IF '(' cond ')' stmt %prec LOWER_THAN_ELSE
                             { $$ = new_if($3, $5, nullptr); }
    | T_IF '(' cond ')' stmt T_ELSE stmt
                             { $$ = new_if($3, $5, $7); }
    | T_WHILE '(' cond ')' stmt
                             { $$ = new_while($3, $5); }
    | '{' stmt_list '}'       { $$ = $2; }
    ;

cond
    : expr '<' expr           { $$ = new_binop('<', $1, $3); }
    | expr '>' expr           { $$ = new_binop('>', $1, $3); }
    | expr T_LE expr          { $$ = new_binop('L', $1, $3); }
    | expr T_GE expr          { $$ = new_binop('G', $1, $3); }
    | expr T_EQ expr          { $$ = new_binop('E', $1, $3); }
    | expr T_NE expr          { $$ = new_binop('N', $1, $3); }
    ;

expr
    : T_NUM                   { $$ = new_num($1); }
    | T_ID                    { $$ = new_var(*$1); delete $1; }
    | expr '+' expr           { $$ = new_binop('+', $1, $3); }
    | expr '-' expr           { $$ = new_binop('-', $1, $3); }
    | expr '*' expr           { $$ = new_binop('*', $1, $3); }
    | expr '/' expr           { $$ = new_binop('/', $1, $3); }
    | '(' expr ')'            { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Parse error: " << s << std::endl;
}
