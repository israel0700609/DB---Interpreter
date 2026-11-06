#include "ast.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable *build_symbol_table(ASTNode *root);

void add_columns(Table *table, ASTNode *columns);

int type_checking(ColumnType left, ColumnType right);

ColumnType evaluate_expression(Table *current_table, ASTNode *expression);

ColumnType analyze_expression(Table *current_table, ASTNode *expression);
