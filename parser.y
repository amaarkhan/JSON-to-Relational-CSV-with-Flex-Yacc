%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

// Function declarations
ASTNode *make_object(ASTNode *pairs);
ASTNode *make_array(ASTNode *values);
ASTNode *make_string(char *value);
ASTNode *make_number(double value);
ASTNode *make_bool(int value);
ASTNode *make_null();
ASTNode *make_pair(char *key, ASTNode *value);
ASTNode *make_pair_list(ASTNode *pair, ASTNode *next);
ASTNode *make_array_list(ASTNode *value, ASTNode *next);

extern int yylex();
void yyerror(const char *s);

extern ASTNode *ast_root;
%}

%code requires {
    #include "ast.h"
}

// Union to define yylval types
%union {
    char *str_val;      // For STRING tokens and string values
    double num_val;     // For NUMBER tokens
    int boolean;        // For TRUE/FALSE
    ASTNode *node_val;  // For AST nodes
}

// Token declarations
%token <str_val> STRING
%token <num_val> NUMBER
%token <boolean> BOOLEAN
%token TRUE FALSE NULLVAL
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA

// Map TRUE/FALSE tokens to BOOLEAN type
%type <node_val> json value object array members elements pair

%%

json:
    value               { ast_root = $1; }
;

value:
      object            { $$ = $1; }
    | array             { $$ = $1; }
    | STRING            { $$ = make_string($1); }
    | NUMBER            { $$ = make_number($1); }
    | TRUE              { $$ = make_bool(1); }
    | FALSE             { $$ = make_bool(0); }
    | NULLVAL           { $$ = make_null(); }
;

object:
      LBRACE members RBRACE    { $$ = make_object($2); }
    | LBRACE RBRACE            { $$ = make_object(NULL); }
;

members:
      pair                     { $$ = make_pair_list($1, NULL); }
    | pair COMMA members       { $$ = make_pair_list($1, $3); }
;

pair:
    STRING COLON value         { $$ = make_pair($1, $3); }
;

array:
      LBRACKET elements RBRACKET    { $$ = make_array($2); }
    | LBRACKET RBRACKET            { $$ = make_array(NULL); }
;

elements:
      value                     { $$ = make_array_list($1, NULL); }
    | value COMMA elements      { $$ = make_array_list($1, $3); }
;

%%

void yyerror(const char *s) {
    extern int line_num, col_num;
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, line_num, col_num);
    exit(EXIT_FAILURE);
}
