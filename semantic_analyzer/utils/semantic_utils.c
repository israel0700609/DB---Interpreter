#include "semantic_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char *current_filename;

const char *column_type_to_string(ColumnType type) {
  switch (type) {
  case INT:
    return "INT";
  case FLOAT:
    return "FLOAT";
  case TEXT:
    return "TEXT";
  case BOOL:
    return "BOOL";
  case NULL_TYPE:
    return "NULL_TYPE";
  default:
    return "UNKNOWN";
  }
}

int is_valid_column_type(char *type_str) {
  if (type_str == NULL) {
    return 0;
  }
  return (strcmp(type_str, "INT") == 0 || strcmp(type_str, "FLOAT") == 0 ||
          strcmp(type_str, "BOOL") == 0 || strcmp(type_str, "TEXT") == 0);
}

int type_checking(ColumnType left, ColumnType right) {
  if (left == right) {
    return 1;
  }
  if ((left == INT && right == FLOAT) || (left == FLOAT && right == INT)) {
    return 1;
  }
  if ((left == BOOL && right == INT) || (left == INT && right == BOOL)) {
    return 1;
  }
  return 0;
}

ColumnType analyze_function_call(Table *table, ASTNode *func_node) {
  char *func_name = func_node->data.func.name;
  ASTNode *arg = func_node->data.func.arg;

  int is_star = 0;
  if (arg != NULL && arg->type == NODE_IDENTIFIER &&
      strcmp(arg->data.str_val, "*") == 0) {
    is_star = 1;
  }

  if (is_star) {
    if (strcmp(func_name, "COUNT") == 0) {
      return INT;
    } else {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Function '%s' cannot take "
              "'*' as an argument.\n",
              current_filename, func_node->lineno, func_name);
      exit(1);
    }
  }

  ColumnType arg_type = evaluate_expression(table, arg);

  if (strcmp(func_name, "SUM") == 0 || strcmp(func_name, "AVG") == 0) {
    if (arg_type != INT && arg_type != FLOAT) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Function '%s' requires "
              "numeric argument.\n",
              current_filename, func_node->lineno, func_name);
      exit(1);
    }
    return arg_type;
  }

  else if (strcmp(func_name, "COUNT") == 0 || strcmp(func_name, "MAX") == 0 ||
           strcmp(func_name, "MIN") == 0) {
    if (strcmp(func_name, "COUNT") == 0)
      return INT;
    return arg_type;
  }

  else {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: Unknown function '%s'.\n",
            current_filename, func_node->lineno, func_name);
    exit(1);
  }
}

void add_columns(Table *table, ASTNode *columns) {
  if (!columns) {
    fprintf(stderr,
            "Semantic Error in '%s': Table '%s' has no columns defined.\n",
            current_filename, table->name);
    exit(1);
  }

  int has_primary_key = 0;
  ASTNode *current = columns;

  while (current) {
    ColumnDef colDef = current->data.col_def;

    if (colDef.is_primary_key) {
      if (has_primary_key) {
        fprintf(stderr,
                "Semantic Error in '%s': Table '%s' cannot have multiple "
                "primary keys (conflict on column '%s').\n",
                current_filename, table->name, colDef.name);
        exit(1);
      }
      has_primary_key = 1;
    }

    if (!is_valid_column_type(colDef.data_type)) {
      fprintf(stderr,
              "Semantic Error in '%s': Unknown data type '%s' for column '%s' "
              "in table '%s'.\n",
              current_filename, colDef.data_type ? colDef.data_type : "NULL",
              colDef.name, table->name);
      exit(1);
    }

    ColumnType cType = string_to_column_type(colDef.data_type);

    if (!add_column_to_table(table, colDef.name, cType, colDef.is_primary_key,
                             colDef.is_not_null)) {
      fprintf(
          stderr,
          "Semantic Error in '%s': Duplicate column name '%s' in table '%s'.\n",
          current_filename, colDef.name, table->name);
      exit(1);
    }

    current = current->next;
  }
}

SymbolTable *build_symbol_table(ASTNode *root, SymbolTable *existing_st) {
  if (!root) {
    return existing_st;
  }

  SymbolTable *symbol_table = existing_st ? existing_st : create_symbol_table();

  ASTNode *current = root;
  if (current->type == NODE_PROGRAM) {
    current = current->data.program.stmts;
  }

  while (current) {
    if (current->type == NODE_CREATE_STMT) {
      char *currentName = current->data.create_stmt.table_name;

      if (find_table(symbol_table, currentName)) {
        fprintf(stderr,
                "Semantic Error in '%s': Could not create table '%s' (table "
                "already exists).\n",
                current_filename, currentName);
        exit(1);
      }

      Table *table = add_table(symbol_table, currentName);
      if (!table) {
        fprintf(stderr,
                "Semantic Error in '%s': Could not create table '%s'.\n",
                current_filename, currentName);
        exit(1);
      }

      add_columns(table, current->data.create_stmt.column_list);
    }
    current = current->next;
  }

  return symbol_table;
}

ColumnType analyze_expression(Table *current_table, ASTNode *expression);

ColumnType evaluate_expression(Table *current_table, ASTNode *expression) {
  if (!expression) {
    return NULL_TYPE;
  }

  switch (expression->type) {
  case NODE_IDENTIFIER: {
    char *col_name = expression->data.col_def.name;
    Column *col = find_column(current_table, col_name);
    if (!col) {
      fprintf(
          stderr,
          "Semantic Error in '%s': Could not find column '%s' in table '%s'.\n",
          current_filename, col_name, current_table->name);
      exit(1);
    }
    return col->type;
  }

  case NODE_INT_LITERAL:
    return INT;
  case NODE_REAL_LITERAL:
    return FLOAT;
  case NODE_STRING_LITERAL:
    return TEXT;
  case NODE_BOOL_LITERAL:
    return BOOL;
  case NODE_NULL_LITERAL:
    return NULL_TYPE;

  case NODE_BINARY_EXPR:
  case NODE_UNARY_EXPR:
  case NODE_ASSIGNMENT:
    return analyze_expression(current_table, expression);

  case NODE_FUNC_CALL:
    return analyze_function_call(current_table, expression);

  default:
    fprintf(stderr, "Semantic Error in '%s': Unknown expression node type.\n",
            current_filename);
    exit(1);
  }
}

ColumnType analyze_expression(Table *current_table, ASTNode *expression) {
  if (!expression) {
    return NULL_TYPE;
  }

  if (expression->type == NODE_BINARY_EXPR) {
    ColumnType left =
        evaluate_expression(current_table, expression->data.binary.left);
    ColumnType right =
        evaluate_expression(current_table, expression->data.binary.right);
    char *op = expression->data.binary.op;

    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || strcmp(op, "*") == 0 ||
        strcmp(op, "/") == 0) {

      if ((left != INT && left != FLOAT && left != NULL_TYPE) ||
          (right != INT && right != FLOAT && right != NULL_TYPE)) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Type Mismatch - left "
                "type(%s) op(%s) right type(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(left), op, column_type_to_string(right));
        exit(1);
      }

      if (left == NULL_TYPE || right == NULL_TYPE)
        return NULL_TYPE;
      if (left == FLOAT || right == FLOAT)
        return FLOAT;
      return INT;

    } else if (strcmp(op, "%") == 0) {
      if (left != INT || right != INT) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Modulo allows only INT - "
                "left(%s) op(%s) right(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(left), op, column_type_to_string(right));
        exit(1);
      }
      return INT;

    } else if (strcmp(op, "AND") == 0 || strcmp(op, "OR") == 0) {
      if ((left != INT && left != BOOL && left != NULL_TYPE) ||
          (right != INT && right != BOOL && right != NULL_TYPE)) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Type Mismatch - left(%s) "
                "op(%s) right(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(left), op, column_type_to_string(right));
        exit(1);
      }
      return BOOL;

    } else if (strcmp(op, "CONCAT") == 0) {
      if (left != TEXT || right != TEXT) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Type Mismatch - left(%s) "
                "op(%s) right(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(left), op, column_type_to_string(right));
        exit(1);
      }
      return TEXT;

    } else {
      if (!type_checking(left, right)) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Type Mismatch - left(%s) "
                "op(%s) right(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(left), op, column_type_to_string(right));
        exit(1);
      }
      return BOOL;
    }
  }

  else if (expression->type == NODE_UNARY_EXPR) {
    ColumnType operand =
        evaluate_expression(current_table, expression->data.unary.operand);
    char *op = expression->data.unary.op;

    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) {
      if (operand != INT && operand != FLOAT) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Type Mismatch - op(%s) "
                "operand(%s).\n",
                current_filename, expression->lineno, op,
                column_type_to_string(operand));
        exit(1);
      }
      return operand;

    } else if (strcmp(op, "!") == 0) {
      if (operand != BOOL) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: '!' allows only BOOL - "
                "operand(%s).\n",
                current_filename, expression->lineno,
                column_type_to_string(operand));
        exit(1);
      }
      return BOOL;

    } else if (strcmp(op, "IS_NULL") == 0 || strcmp(op, "IS_NOT_NULL") == 0) {
      return BOOL;
    }
  }

  else if (expression->type == NODE_ASSIGNMENT) {
    ColumnType lhs = evaluate_expression(
        current_table, expression->data.assignment.identifier);
    ColumnType rhs =
        evaluate_expression(current_table, expression->data.assignment.expr);

    if (!type_checking(lhs, rhs)) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Type Mismatch - lhs(%s) "
              "rhs(%s).\n",
              current_filename, expression->lineno, column_type_to_string(lhs),
              column_type_to_string(rhs));
      exit(1);
    }
    return lhs;
  }
  return NULL_TYPE;
}

int has_aggregate_function(ASTNode *columns) {
  ASTNode *current = columns;
  while (current) {
    if (current->type == NODE_FUNC_CALL) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

int check_aggregate_query(ASTNode *group_by, ASTNode *columns) {
  return (group_by != NULL) || has_aggregate_function(columns);
}

int is_literal(ASTNode *column) {
  NodeType type = column->type;
  return (type == NODE_INT_LITERAL) || (type == NODE_BOOL_LITERAL) ||
         (type == NODE_REAL_LITERAL) || (type == NODE_STRING_LITERAL) ||
         (type == NODE_NULL_LITERAL);
}

int is_in_group_by(ASTNode *group_by, ASTNode *id) {
  ASTNode *current = group_by;
  while (current) {
    if (current->type == NODE_IDENTIFIER) {
      if (strcmp(current->data.str_val, id->data.str_val) == 0) {
        return 1;
      }
    }
    current = current->next;
  }
  return 0;
}

int validate_aggregate_column(ASTNode *group_by, ASTNode *column) {
  if (is_literal(column)) {
    return 1;
  }
  if (column->type == NODE_FUNC_CALL) {
    return 1;
  }
  if (column->type == NODE_IDENTIFIER) {
    return is_in_group_by(group_by, column);
  }
  return 0;
}

void validate_where(Table *table, ASTNode *where_caluse) {
  if (!validate_where_clause(where_caluse)) {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: Aggregate functions are not "
            "allowed in WHERE clause.\n",
            current_filename, where_caluse->lineno);
    exit(1);
  }

  ColumnType type = evaluate_expression(table, where_caluse);
  if (type != BOOL && type != INT) {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: Type Mismatch at WHERE clause. "
            "Expected BOOL/INT, got %s.\n",
            current_filename, where_caluse->lineno,
            column_type_to_string(type));
    exit(1);
  }
}

int validate_where_clause(ASTNode *clause) {
  if (!clause)
    return 1;

  if (is_literal(clause))
    return 1;

  if (clause->type == NODE_FUNC_CALL) {
    return 0;
  }

  if (clause->type == NODE_BINARY_EXPR) {
    return validate_where_clause(clause->data.binary.left) &&
           validate_where_clause(clause->data.binary.right);
  }
  if (clause->type == NODE_UNARY_EXPR) {
    return validate_where_clause(clause->data.unary.operand);
  }

  return 1;
}

int validate_having_clause(ASTNode *group_by, ASTNode *clause) {
  if (!clause)
    return 1;

  if (is_literal(clause))
    return 1;

  if (clause->type == NODE_FUNC_CALL) {
    return 1;
  }

  if (clause->type == NODE_IDENTIFIER && !is_in_group_by(group_by, clause)) {
    return 0;
  }

  if (clause->type == NODE_BINARY_EXPR) {
    return validate_having_clause(group_by, clause->data.binary.left) &&
           validate_having_clause(group_by, clause->data.binary.right);
  }
  if (clause->type == NODE_UNARY_EXPR) {
    return validate_having_clause(group_by, clause->data.unary.operand);
  }

  return 1;
}

void check_select_semantics(ASTNode *select_node, SymbolTable *symbol_table) {
  SelectStmt select_stmt = select_node->data.select_stmt;
  char *tableName = select_stmt.from_table;
  ASTNode *columns = select_stmt.select_list;
  ASTNode *whereClause = select_stmt.where_clause;
  ASTNode *groupBy = select_stmt.group_by;
  ASTNode *having = select_stmt.having;
  ASTNode *orderBy = select_stmt.order_by;
  int limitVal = select_stmt.limit_val;
  Table *table = find_table(symbol_table, tableName);
  if (!table) {
    fprintf(stderr, "Semantic Error in '%s': Table '%s' does not exist.\n",
            current_filename, tableName);
    exit(1);
  }

  ASTNode *current = columns;
  int is_aggregate_query = check_aggregate_query(groupBy, columns);

  while (current) {
    if (current->type == NODE_IDENTIFIER &&
        strcmp(current->data.str_val, "*") == 0) {
      if (is_aggregate_query) {
        fprintf(stderr,
                "Semantic Error in '%s': Cannot use '*' in SELECT list with "
                "GROUP BY.\n",
                current_filename);
        exit(1);
      }
      current = current->next;
      continue;
    }
    if (is_aggregate_query) {
      if (!validate_aggregate_column(groupBy, current)) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Column '%s' must appear in "
                "GROUP BY or be used in an aggregate function.\n",
                current_filename, current->lineno, current->data.str_val);
        exit(1);
      }
    }

    ColumnType type = evaluate_expression(table, current);

    if (type == NULL_TYPE && current->type != NODE_NULL_LITERAL) {
      fprintf(
          stderr,
          "Semantic Error in '%s' at line %d: Invalid expression in SELECT.\n",
          current_filename, current->lineno);
      exit(1);
    }
    current = current->next;
  }

  if (whereClause) {
    validate_where(table, whereClause);
  }

  if (groupBy) {
    ASTNode *current_gb = groupBy;
    while (current_gb) {
      evaluate_expression(table, current_gb);

      if (current_gb->type == NODE_FUNC_CALL) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Aggregate function is not "
                "allowed in GROUP BY clause.\n",
                current_filename, current_gb->lineno);
        exit(1);
      }
      current_gb = current_gb->next;
    }
  }

  if (having) {
    if (!validate_having_clause(groupBy, having)) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Column in HAVING clause must "
              "appear in the GROUP BY clause.\n",
              current_filename, having->lineno);
      exit(1);
    }

    ColumnType type = evaluate_expression(table, having);
    if (type != BOOL && type != INT) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Type Mismatch at HAVING "
              "clause. Expected BOOL/INT, got %s.\n",
              current_filename, having->lineno, column_type_to_string(type));
      exit(1);
    }
  }

  if (orderBy) {
    ASTNode *current_ob = orderBy;
    while (current_ob) {
      evaluate_expression(table, current_ob);

      if (is_aggregate_query && !validate_having_clause(groupBy, current_ob)) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Column in ORDER BY must "
                "appear in the GROUP BY clause.\n",
                current_filename, current_ob->lineno);
        exit(1);
      }
      current_ob = current_ob->next;
    }
  }

  if (limitVal < 0 && limitVal != -1) {
    fprintf(stderr,
            "Semantic Error in '%s': Limit val must be a non-negative integer "
            "value for table '%s'.\n",
            current_filename, table->name);
    exit(1);
  }
}

int count_ast_nodes(ASTNode *node_list) {
  int count = 0;
  ASTNode *current = node_list;
  while (current) {
    count++;
    current = current->next;
  }
  return count;
}

int count_columns(Column *columns) {
  int count = 0;
  Column *current = columns;
  while (current) {
    count++;
    current = current->next;
  }
  return count;
}

void check_is_null_value(Column *column, ColumnType value_type, int lineno) {
  if (column->is_not_null && value_type == NULL_TYPE) {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: NULL is not allowed. Column "
            "'%s' has NOT NULL constraint.\n",
            current_filename, lineno, column->name);
    exit(1);
  }
}

void validate_non_null_columns(Column *columns, ASTNode *column_list,
                               int lineno) {
  Column *tableCol = columns;
  while (tableCol) {
    if (tableCol->is_not_null) {
      ASTNode *currCol = column_list;
      int found = 0;
      while (currCol) {
        if (currCol->type == NODE_IDENTIFIER &&
            strcmp(currCol->data.str_val, tableCol->name) == 0) {
          found = 1;
          break;
        }
        currCol = currCol->next;
      }
      if (!found) {
        fprintf(stderr,
                "Semantic Error in '%s' at line %d: Field '%s' has a NOT NULL "
                "constraint and is missing from INSERT column list.\n",
                current_filename, lineno, tableCol->name);
        exit(1);
      }
    }
    tableCol = tableCol->next;
  }
}

Column *convert_ast_column_to_Columns(Table *table, ASTNode *columns) {
  Column *head = (Column *)malloc(sizeof(Column));
  if (!head) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  head->next = NULL;

  ASTNode *current = columns;
  Column *currentHead = head;

  while (current) {
    Column *original_col = find_column(table, current->data.str_val);
    if (!original_col) {
      fprintf(stderr,
              "Semantic Error in '%s': Column '%s' not found in table '%s'.\n",
              current_filename, current->data.str_val, table->name);
      exit(1);
    }

    Column *new_col = (Column *)malloc(sizeof(Column));
    if (!new_col) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
    }

    new_col->name = strdup(original_col->name);
    new_col->type = original_col->type;
    new_col->is_primary_key = original_col->is_primary_key;
    new_col->is_not_null = original_col->is_not_null;
    new_col->next = NULL;

    currentHead->next = new_col;
    currentHead = currentHead->next;
    current = current->next;
  }

  Column *real_head = head->next;
  free(head);
  return real_head;
}

void validate_values_columns(Table *table, Column *columns,
                             ASTNode *values_list) {
  Column *currentTableCol = columns;
  ASTNode *valuesCurrent = values_list;

  while (currentTableCol && valuesCurrent) {
    ColumnType valuesType = evaluate_expression(table, valuesCurrent);

    check_is_null_value(currentTableCol, valuesType, values_list->lineno);

    if (!type_checking(currentTableCol->type, valuesType)) {
      fprintf(
          stderr,
          "Semantic Error in '%s' at line %d: Type Mismatch (Insert "
          "statement). Column '%s' expects type(%s) but got value type(%s).\n",
          current_filename, values_list->lineno, currentTableCol->name,
          column_type_to_string(currentTableCol->type),
          column_type_to_string(valuesType));
      exit(1);
    }

    currentTableCol = currentTableCol->next;
    valuesCurrent = valuesCurrent->next;
  }
}

void validate_values_list(Table *table, Column *column_list,
                          ASTNode *values_list) {

  int columnCount = count_columns(column_list);
  int valuesCount = count_ast_nodes(values_list);
  int countTableCols = count_columns(table->columns);

  if (column_list) {
    if (columnCount != valuesCount) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Insert statement: Mismatch "
              "in columns list. columns count(%d) values count(%d).\n",
              current_filename, values_list->lineno, columnCount, valuesCount);
      exit(1);
    }
    validate_values_columns(table, column_list, values_list);
  } else {
    if (valuesCount != countTableCols) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Insert mismatch. Table '%s' "
              "has %d columns, but %d values provided.\n",
              current_filename, values_list->lineno, table->name,
              countTableCols, valuesCount);
      exit(1);
    }

    validate_values_columns(table, table->columns, values_list);
  }
}

void check_insert_semantics(ASTNode *insert_node, SymbolTable *symbol_table) {
  char *tableName = insert_node->data.insert_stmt.table_name;
  ASTNode *valueList = insert_node->data.insert_stmt.value_list;

  Table *table = find_table(symbol_table, tableName);
  if (!table) {
    fprintf(stderr, "Semantic Error in '%s': Table '%s' does not exist.\n",
            current_filename, tableName);
    exit(1);
  }

  Column *columnList = convert_ast_column_to_Columns(
      table, insert_node->data.insert_stmt.column_list);

  if (!valueList) {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: Insert statement missing "
            "values list.\n",
            current_filename, insert_node->lineno);
    exit(1);
  }

  if (columnList) {
    validate_non_null_columns(table->columns,
                              insert_node->data.insert_stmt.column_list,
                              insert_node->lineno);
    validate_values_list(table, columnList, valueList);
  } else {
    validate_values_list(table, columnList, valueList);
  }
}

int validate_expression_no_aggregates(ASTNode *expr) {
  if (!expr)
    return 1;

  if (expr->type == NODE_FUNC_CALL) {
    return 0;
  }

  if (expr->type == NODE_BINARY_EXPR) {
    return validate_expression_no_aggregates(expr->data.binary.left) &&
           validate_expression_no_aggregates(expr->data.binary.right);
  }

  if (expr->type == NODE_UNARY_EXPR) {
    return validate_expression_no_aggregates(expr->data.unary.operand);
  }

  return 1;
}

void validate_update_list(Table *table, ASTNode *update_list) {
  ASTNode *current = update_list;

  while (current) {
    if (current->type != NODE_ASSIGNMENT) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Expected assignment in "
              "UPDATE SET clause.\n",
              current_filename, current->lineno);
      exit(1);
    }

    ASTNode *id_node = current->data.assignment.identifier;
    ASTNode *expr_node = current->data.assignment.expr;

    char *colName = id_node->data.str_val;
    Column *tableCol = find_column(table, colName);

    if (!validate_expression_no_aggregates(expr_node)) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Aggregate functions are not "
              "allowed in UPDATE SET clause.\n",
              current_filename, current->lineno);
      exit(1);
    }

    ColumnType lhsType = evaluate_expression(table, id_node);
    ColumnType rhsType = evaluate_expression(table, expr_node);

    check_is_null_value(tableCol, rhsType, current->lineno);

    if (!type_checking(lhsType, rhsType)) {
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Type Mismatch (Update "
              "statement). Column '%s' expects type(%s) but got expression "
              "type(%s).\n",
              current_filename, current->lineno, tableCol->name,
              column_type_to_string(lhsType), column_type_to_string(rhsType));
      exit(1);
    }

    current = current->next;
  }
}

void check_update_semantics(ASTNode *update_node, SymbolTable *symbol_table) {
  char *tableName = update_node->data.update_stmt.table_name;
  ASTNode *updateList = update_node->data.update_stmt.update_list;
  ASTNode *whereClause = update_node->data.update_stmt.where_clause;

  Table *table = find_table(symbol_table, tableName);
  if (!table) {
    fprintf(stderr, "Semantic Error in '%s': Table '%s' does not exist.\n",
            current_filename, tableName);
    exit(1);
  }

  if (!updateList) {
    fprintf(stderr,
            "Semantic Error in '%s' at line %d: Update statement missing "
            "update list.\n",
            current_filename, update_node->lineno);
    exit(1);
  }

  validate_update_list(table, updateList);

  if (whereClause) {
    validate_where(table, whereClause);
  }
}

void check_delete_semantics(ASTNode *delete_node, SymbolTable *symbol_table) {
  char *tableName = delete_node->data.delete_stmt.table_name;
  ASTNode *whereClause = delete_node->data.delete_stmt.where_clause;

  Table *table = find_table(symbol_table, tableName);
  if (!table) {
    fprintf(stderr, "Semantic Error in '%s': Table '%s' does not exist.\n",
            current_filename, tableName);
    exit(1);
  }

  if (whereClause) {
    validate_where(table, whereClause);
  }
}

void check_drop_statement(ASTNode *drop_node, SymbolTable *symbol_table) {
  char *tableName = drop_node->data.drop_stmt.table_name;

  Table *table = find_table(symbol_table, tableName);
  if (!table) {
    fprintf(stderr, "Semantic Error in '%s': Table '%s' does not exist.\n",
            current_filename, tableName);
    exit(1);
  }

  drop_table(symbol_table, tableName);
}