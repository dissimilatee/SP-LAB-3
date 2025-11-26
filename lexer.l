%{
#include <string>
#include "ast.h"
#include "parser.tab.h"
using namespace std;
%}

%option noyywrap

DIGIT  [0-9]
ID     [a-zA-Z_][a-zA-Z0-9_]*

%%

"if"       { return T_IF; }
"else"     { return T_ELSE; }
"while"    { return T_WHILE; }
"print"    { return T_PRINT; }

"=="       { return T_EQ; }
"!="       { return T_NE; }
"<="       { return T_LE; }
">="       { return T_GE; }
"<"        { return '<'; }
">"        { return '>'; }

"="        { return '='; }
";"        { return ';'; }
"("        { return '('; }
")"        { return ')'; }
"{"        { return '{'; }
"}"        { return '}'; }
"+"        { return '+'; }
"-"        { return '-'; }
"*"        { return '*'; }
"/"        { return '/'; }

{DIGIT}+   {
              yylval.ival = atoi(yytext);
              return T_NUM;
           }

{ID}       {
              yylval.sval = new std::string(yytext);
              return T_ID;
           }

[ \t\r\n]+  { /* skip whitespace */ }

.          {
              fprintf(stderr, "Unknown character: %s\n", yytext);
           }

%%
