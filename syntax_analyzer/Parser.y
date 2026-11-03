%{

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include "ast/ast.h" 


extern int yylineno;
extern char* yytext; 
extern ASTNode* root; 



int yylex(); 
void yyerror(const char* s);
%}


%union {
    ASTNode* node_val;
    int int_val;
    double real_val;
    char* str_val;
}


/* Keywords */ 
%token CREATE DROP TABLE PRIMARY KEY NOT  
%token SELECT INSERT INTO VALUES UPDATE SET DELETE FROM WHERE  
%token GROUP BY HAVING ORDER ASC DESC LIMIT DISTINCT 
%token UNION INTERSECT MINUS EXISTS

/* Functions */
%token COUNT SUM AVG MIN MAX

/* Data types */
%token TYPE_INT TYPE_TEXT TYPE_FLOAT TYPE_BOOL NULL_TOK
%token <int_val> TRUE_LITERAL FALSE_LITERAL INT_LITERAL 
%token <real_val> REAL_LITERAL 
%token <str_val> STRING_LITERAL

/* Operators & ID */
%token <str_val> IDENTIFIER 
%token EQ NEQ GTE LTE
%token AND OR NOT IN IS



/* Precedence */
%left OR
%left AND
%left EQ NEQ
%left '>' '<' GTE LTE 
%left '+' '-' 
%left '*' '/' 
%right '!'
%right '^'
%right '&'


%%


program: 
        statements { $$ = create_program_node($1, yylineno); root = $$; }
    |              { $$ = create_program_node(NULL, yylineno); root = $$; }
    ;


statements: 
        statements statement { $$ = append_ast_node($1, $2); }
    | 
        statement            { $$ = $1; }
    |                        { $$ = NULL; }
    ;



statement: stmt ';' { $$ = $1; } 
    ; 


stmt:   
        select_stmt { $$ =  $1; }
    | 
        create_stmt { $$ = $1; }
    | 
        insert_stmt { $$ = $1; }
    | 
        update_stmt { $$ = $1; }
    |   
        delete_stmt { $$ = $1; }
    | 
        drop_stmt   { $$ = $1; }
    ; 

select_stmt: 
        select_base_stmt { $$ = $1; }
    | 
        select_stmt select_operator select_base_stmt
        {
            $$ = create_binary_expr_node($2, $1, $3, yylineno);    
        }
    ; 

select_base_stmt:
        SELECT is_distinct select_list FROM IDENTIFIER where_clause group_by having order_by limit
        {
            $$ = create_select_node($2, $3, $5, $6, $7, $8, $9, $10, yylineno);
            free($5);
        }
    ;


is_distinct: 
        DISTINCT { $$ = 1; }
    | 
                 { $$ = 0; }
    ;             


select_list: 
        expression       { $$ = $1; }
    | 
        select_list ',' expression { $$ = append_ast_node($1, $3); }
    ;

where_clause: 
        WHERE expression { $$ = $2; }
    |                    { $$ = NULL; }    
    ;

group_by:   
        GROUP BY group_by_list { $$ = $3; }
    |                          { $$ = NULL; }
    ;

having: 
        HAVING expression      { $$ = $2; }
    |                          { $$ = NULL; }
    ; 

order_by: 
        ORDER BY order_by_list { $$ = $3; }
    |                          { $$ = NULL; }
    ;

limit: 
        LIMIT INT_LITERAL      { $$ = $2; }
    | 
                               { $$ = -1; }
    ;


group_by_list: 
        expression             { $$ = $1; }
    | 
        group_by_list ',' expression { $$ = append_ast_node($1, $3); }
    ;

order_by_list:
        order_item             { $$ = $1; }
    | 
        order_by_list ',' order_item { $$ = append_ast_node($1, $3); }
    ; 

order_item: 
        expression ASC         { $$ = create_order_by_item_node($1, 0, yylineno); }
    | 
        expression DESC        { $$ = create_order_by_item_node($1, 1, yylineno); }
    |
        expression             { $$ = create_order_by_item_node($1, 0, yylineno); }
    ;
    

select_operator: 
        MINUS { $$ = "MINUS"; }
    | 
        UNION { $$ = "UNION"; }
    |   
        INTERSECT { $$ = "INTERSECT"; }
    ;



create_stmt: 
        CREATE TABLE IDENTIFIER '(' column_def_list ')' 
        {
            $$ = create_create_table_node($3, $5, yylineno);
            free($3);
        } 
    ;

column_def_list: 
        column_def { $$ = $1; }
    | 
        column_def_list ',' column_def { $$ = append_ast_node($1, $3); }
    ; 

column_def: 
        IDENTIFIER column_type is_primary_key is_nn 
        {
            $$ = create_column_def_node($1, $2, $3, $4, yylineno);
            free($1);
        }
    ;

column_type: 
        TYPE_INT { $$ = "INT"; }
    | 
        TYPE_TEXT { $$ = "TEXT"; }
    | 
        TYPE_FLOAT { $$ = "FLOAT"; }
    |
        TYPE_BOOL  { $$ = "BOOL"; } 
    ; 

is_primary_key: 
        PRIMARY KEY { $$ = 1; }
    |               { $$ = 0; }
    ; 

is_nn: 
        NOT NULL_TOK { $$ = 1; }
    |                { $$ = 0; }
    ;


insert_stmt: 
        INSERT INTO IDENTIFIER insert_order VALUES '(' values_list ')'
        {
            $$ = create_insert_table_node($3, $4, $7, yylineno);
            free($3);
        }
    ;

insert_order: 
        '(' insert_order_list ')' { $$ = $2; }
    |                             { $$ = NULL; }
    ; 

insert_order_list: 
        IDENTIFIER 
        { 
            $$ = create_identifier_node($1, yylineno);
            free($1);
        }
    | 
        insert_order_list ',' IDENTIFIER 
        { 
            ASTNode *id_node = create_identifier_node($3, yylineno);
            $$ = append_ast_node($1, id_node); 
            free($3);
        }
    ;

values_list: 
        expression { $$ = $1; }
    | 
        values_list ',' expression { $$ = append_ast_node($1, $3); }
    ;

update_stmt: 
        UPDATE IDENTIFIER SET update_list where_clause
        {
            $$ = create_update_table_node($2, $4, $5, yylineno);
            free($2);
        }
    ;


update_list: 
        assignment { $$ = $1; }
    | 
        update_list ',' assignment { $$ = append_ast_node($1, $3); }
    ; 

assignment: 
        IDENTIFIER EQ expression
        {
            $$ = create_assignment_node($1, $3, yylineno);
            free($1);
        }
    ;

delete_stmt: 
        DELETE FROM IDENTIFIER where_clause
        {
            $$ = create_delete_table_node($3, $4, yylineno);
            free($3);
        }
    ;


drop_stmt: 
        DROP TABLE IDENTIFIER 
        {
            $$ = create_drop_table_node($3, yylineno);
            free($3);
        }
    ;
%% 