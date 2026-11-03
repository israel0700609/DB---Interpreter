#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {

  NODE_SELECT_STMT,
  NODE_CREATE_STMT,
  NODE_INSERT_STMT,
  NODE_UPDATE_STMT,
  NODE_DELETE_STMT,
  NODE_DROP_STMT,

  NODE_COLUMN_DEF,
  NODE_ORDER_BY_ITEM,

  NODE_BINARY_EXPR,
  NODE_ASSIGNMENT,
  NODE_UNARY_EXPR,
  NODE_FUNC_CALL,

  NODE_IDENTIFIER,
  NODE_INT_LITERAL,
  NODE_REAL_LITERAL,
  NODE_STRING_LITERAL,
  NODE_BOOL_LITERAL,
  NODE_NULL_LITERAL,

  NODE_PROGRAM
} NodeType;

struct ASTNode;

typedef struct {
  char *op;
  struct ASTNode *left;
  struct ASTNode *right;
} BinaryExpr;

typedef struct {
  struct ASTNode *identifier;
  struct ASTNode *expr;
} Assignment;

typedef struct {
  char *op;
  struct ASTNode *operand;
} UnaryExpr;

typedef struct {
  char *name;
  struct ASTNode *arg;
} FuncCall;

typedef struct {
  char *name;
  char *data_type;
  int is_primary_key;
  int is_not_null;
} ColumnDef;

typedef struct {
  struct ASTNode *expr;
  int is_desc;
} OrderByItem;

typedef struct {
  int is_distinct;
  struct ASTNode *select_list;
  char *from_table;
  struct ASTNode *where_clause;
  struct ASTNode *group_by;
  struct ASTNode *having;
  struct ASTNode *order_by;
  int limit_val;
} SelectStmt;

typedef struct {
  char *table_name;
  struct ASTNode *column_list;
} CreateStmt;

typedef struct {
  char *table_name;
  struct ASTNode *column_list;
  struct ASTNode *value_list;
} InsertStmt;

typedef struct {
  char *table_name;
  struct ASTNode *update_list;
  struct ASTNode *where_clause;
} UpdateStmt;

typedef struct {
  char *table_name;
  struct ASTNode *where_clause;
} DeleteStmt;

typedef struct {
  char *table_name;
} DropStmt;

typedef struct {
  struct ASTNode *stmts;
} ProgramNode;

typedef struct ASTNode {
  NodeType type;
  int lineno;

  union {
    int int_val;
    double real_val;
    char *str_val;

    BinaryExpr binary;
    Assignment assignment;
    UnaryExpr unary;
    FuncCall func;

    SelectStmt select_stmt;
    CreateStmt create_stmt;
    InsertStmt insert_stmt;
    UpdateStmt update_stmt;
    DeleteStmt delete_stmt;
    DropStmt drop_stmt;
    ColumnDef col_def;

    OrderByItem order_by_item;

    ProgramNode program;
  } data;

  struct ASTNode *next;
} ASTNode;

ASTNode *create_int_node(int val, int lineno);
ASTNode *create_real_node(double val, int lineno);
ASTNode *create_string_node(char *val, int lineno);
ASTNode *create_bool_node(int val, int lineno);
ASTNode *create_null_node(int lineno);
ASTNode *create_identifier_node(char *name, int lineno);

ASTNode *create_binary_expr_node(char *op, ASTNode *left, ASTNode *right,
                                 int lineno);
ASTNode *create_unary_expr_node(char *op, ASTNode *operand, int lineno);

ASTNode *create_func_call_node(char *name, ASTNode *arg, int lineno);

ASTNode *create_select_node(int is_distinct, ASTNode *select_list,
                            char *table_name, ASTNode *where_clause,
                            ASTNode *group_by, ASTNode *having,
                            ASTNode *order_by, int limit_val, int lineno);

ASTNode *create_create_table_node(char *table_name, ASTNode *column_list,
                                  int lineno);

ASTNode *create_insert_table_node(char *table_name, ASTNode *column_list,
                                  ASTNode *value_list, int lineno);

ASTNode *create_update_table_node(char *table_name, ASTNode *update_list,
                                  ASTNode *where_clause, int lineno);

ASTNode *create_delete_table_node(char *table_name, ASTNode *where_clause,
                                  int lineno);

ASTNode *create_drop_table_node(char *table_name, int lineno);

ASTNode *create_column_def_node(char *name, char *type, int is_pk, int is_nn,
                                int lineno);

ASTNode *create_assignment_node(ASTNode *identifier, ASTNode *expr, int lineno);

ASTNode *create_order_by_item_node(ASTNode *expr, int is_desc, int lineno);

ASTNode *create_program_node(ASTNode *stmts, int lineno);

ASTNode *append_ast_node(ASTNode *head, ASTNode *new_node);

void print_ast(ASTNode *node, int indent);

void free_ast(ASTNode *node);

#endif