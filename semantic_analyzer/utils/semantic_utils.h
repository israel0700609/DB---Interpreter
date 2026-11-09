#ifndef SEMANTIC_UTILS_H
#define SEMANTIC_UTILS_H

#include "ast/ast.h"
#include "model/symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *column_type_to_string(ColumnType type);
int is_valid_column_type(char *type_str);
int type_checking(ColumnType left, ColumnType right);
int is_literal(ASTNode *column);
int count_ast_nodes(ASTNode *node_list);
int count_columns(Column *columns);

ColumnType evaluate_expression(Table *current_table, ASTNode *expression);
ColumnType analyze_expression(Table *current_table, ASTNode *expression);
ColumnType analyze_function_call(Table *table, ASTNode *func_node);
int validate_expression_no_aggregates(ASTNode *expr);

SymbolTable *build_symbol_table(ASTNode *root, SymbolTable *symbol_table);
void add_columns(Table *table, ASTNode *columns);

int has_aggregate_function(ASTNode *columns);
int check_aggregate_query(ASTNode *group_by, ASTNode *columns);
int is_in_group_by(ASTNode *group_by, ASTNode *id);
int validate_aggregate_column(ASTNode *group_by, ASTNode *column);
void validate_where(Table *table, ASTNode *where_caluse);
int validate_where_clause(ASTNode *clause);
int validate_having_clause(ASTNode *group_by, ASTNode *clause);

void check_select_semantics(ASTNode *select_node, SymbolTable *symbol_table);

void check_is_null_value(Column *column, ColumnType value_type, int lineno);
void validate_non_null_columns(Column *columns, ASTNode *column_list,
                               int lineno);
Column *convert_ast_column_to_Columns(Table *table, ASTNode *columns);
void validate_values_columns(Table *table, Column *columns,
                             ASTNode *values_list);
void validate_values_list(Table *table, Column *column_list,
                          ASTNode *values_list);
void check_insert_semantics(ASTNode *insert_node, SymbolTable *symbol_table);

void validate_update_list(Table *table, ASTNode *update_list);
void check_update_semantics(ASTNode *update_node, SymbolTable *symbol_table);

void check_delete_semantics(ASTNode *delete_node, SymbolTable *symbol_table);
void check_drop_statement(ASTNode *drop_node, SymbolTable *symbol_table);

#endif