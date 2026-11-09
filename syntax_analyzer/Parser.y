%{

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include "ast/ast.h" 
#include "semantic_analyzer/analyzer.h"

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
%token CREATE DROP TABLE PRIMARY KEY 
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
%token EQ NEQ GTE LTE CONCAT
%token AND OR NOT IN IS

%type <node_val> program statements statement stmt select_stmt select_base_stmt select_list where_clause group_by having order_by group_by_list order_by_list order_item create_stmt column_def_list column_def insert_stmt insert_order insert_order_list values_list update_stmt update_list assignment delete_stmt drop_stmt expression 

%type <int_val> is_distinct limit is_primary_key is_nn

%type <str_val> select_operator column_type aggregation_function


/* Precedence */
%left UNION INTERSECT MINUS
%left OR
%left AND
%left EQ NEQ
%left '>' '<' GTE LTE IS CONCAT
%left '+' '-' '%'
%left '*' '/'
%right UPLUS UMINUS 
%right NOT '!'


%%


program: 
        statements { $$ = create_program_node($1, yylineno); root = $$; }
    |              { $$ = create_program_node(NULL, yylineno); root = $$; }
    ;


statements: 
        statements statement { $$ = append_ast_node($1, $2); }
    | 
        statement            { $$ = $1; }
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
    | 
        '*' { $$ = create_identifier_node("*", yylineno); }
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
        ASTNode* id_node = create_identifier_node($1, yylineno);
        
        $$ = create_assignment_node(id_node, $3, yylineno);
        
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


expression: 
        expression OR expression 
        { 
            $$ = create_binary_expr_node("OR", $1, $3, yylineno); 
        }
    | 
        expression AND expression
        {
            $$ = create_binary_expr_node("AND", $1, $3, yylineno);
        }
    | 
        expression EQ expression 
        {
            $$ = create_binary_expr_node("=", $1, $3, yylineno);
        }
    | 
        expression NEQ expression 
        {
            $$ = create_binary_expr_node("NEQ", $1, $3, yylineno);
        }
    |
        expression GTE expression 
        {
            $$ = create_binary_expr_node("GTE", $1, $3, yylineno);
        }
    |
        expression LTE expression 
        {
            $$ = create_binary_expr_node("LTE", $1, $3, yylineno);
        }
    |
        expression CONCAT expression 
        {
            $$ = create_binary_expr_node("CONCAT", $1, $3, yylineno);
        }
    |
        expression '+' expression 
        {
            $$ = create_binary_expr_node("+", $1, $3, yylineno);
        }
    |
        expression '-' expression 
        {
            $$ = create_binary_expr_node("-", $1, $3, yylineno);
        } 
    |
        expression '/' expression 
        {
            $$ = create_binary_expr_node("/", $1, $3, yylineno);
        }
    |
        expression '*' expression 
        {
            $$ = create_binary_expr_node("*", $1, $3, yylineno);
        }
    |
        expression '%' expression 
        {
            $$ = create_binary_expr_node("%", $1, $3, yylineno);
        }
    |
        expression '<' expression 
        {
            $$ = create_binary_expr_node("<", $1, $3, yylineno);
        }
    |
        expression '>' expression
        {
            $$ = create_binary_expr_node(">", $1, $3, yylineno);
        }
    | 
        '+' expression %prec UPLUS
            { 
                $$ = create_unary_expr_node("+", $2, yylineno); 
            }
    |
        '-' expression  %prec UMINUS
        { 
            $$ = create_unary_expr_node("-", $2, yylineno); 
        }
    |   
        '!' expression { $$ = create_unary_expr_node("!", $2, yylineno); }
    |
        NOT expression { $$ = create_unary_expr_node("!", $2, yylineno); }
    |
        expression IS NULL_TOK
        { 
            $$ = create_unary_expr_node("IS_NULL", $1, yylineno); 
        }
    | 
        expression IS NOT NULL_TOK 
        {
            $$ = create_unary_expr_node("IS_NOT_NULL", $1, yylineno);
        }
    |
        '(' expression ')' { $$ = $2; }
    |
        aggregation_function '(' expression ')'
        {
            $$ = create_func_call_node($1, $3, yylineno);
        }
    |
        COUNT '(' '*' ')' 
        { 
            ASTNode *star_node = create_identifier_node("*", yylineno);
            $$ = create_func_call_node("COUNT", star_node, yylineno); 
        }    
    |
        COUNT '(' expression ')'
        {
            $$ = create_func_call_node("COUNT", $3, yylineno);
        }
    |
        IDENTIFIER 
        {
            $$ = create_identifier_node($1, yylineno);
            free($1);
        }
    | 
        TRUE_LITERAL { $$ = create_bool_node(1,yylineno); }
    |
        FALSE_LITERAL { $$ = create_bool_node(0, yylineno); }
    | 
        INT_LITERAL { $$ = create_int_node($1, yylineno); }
    |
        REAL_LITERAL { $$ = create_real_node($1, yylineno); }
    | 
        STRING_LITERAL { $$ = create_string_node($1, yylineno); }
    | 
        NULL_TOK { $$ = create_null_node(yylineno); }
    
    ; 

aggregation_function: 
        SUM   { $$ = "SUM"; }
    |
        AVG   { $$ = "AVG"; }
    | 
        MIN   { $$ = "MIN"; }
    |   
        MAX   { $$ = "MAX"; }
    ;

%% 







void yyerror(const char* s){
    printf("Syntax Error on line %d: %s: Unexpected token '%s' \n ", yylineno, s, yytext);
}

int main(int argc, char** argv){
    FILE* out = stdout;

    if(argc > 1){
        out = fopen(argv[1], "w");
        if(!out){
            fprintf(stderr, "Failed to open file: %s\n", argv[1]);
            return 1;
        }
    }

    if(yyparse() == 0){
        fprintf(out, " Parsing Successful. \n");
        fprintf(out, " Abstract Syntax Tree: \n");

        print_ast(root, out, 0);
    }

    run_semantics(root);

    fprintf(out, "Semantics analyzing successful.\n");

    if(out != stdout){
        fclose(out);
    }


    free_ast(root);
    
    return 0; 
}
