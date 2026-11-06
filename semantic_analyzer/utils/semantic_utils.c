#include "semantic_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void add_columns(Table *table, ASTNode *columns) {
  if (!columns) {
    fprintf(stderr, "Semantic Error: Table %s has no columns defined.\n",
            table->name);
    exit(1);
  }

  int has_primary_key = 0;
  ASTNode *current = columns;

  while (current) {
    ColumnDef colDef = current->data.col_def;

    if (colDef.is_primary_key) {
      if (has_primary_key) {
        fprintf(stderr,
                "Semantic Error: Table '%s' cannot have multiple primary keys "
                "(conflict on column '%s').\n",
                table->name, colDef.name);
        exit(1);
      }
      has_primary_key = 1;
    }

    if (!is_valid_column_type(colDef.data_type)) {
      fprintf(stderr,
              "Semantic Error: Unknown data type '%s' for column '%s' in table "
              "'%s'.\n",
              colDef.data_type ? colDef.data_type : "NULL", colDef.name,
              table->name);
      exit(1);
    }

    ColumnType cType = string_to_column_type(colDef.data_type);

    if (!add_column_to_table(table, colDef.name, cType, colDef.is_primary_key,
                             colDef.is_not_null)) {
      fprintf(stderr,
              "Semantic Error: Duplicate column name '%s' in table '%s'.\n",
              colDef.name, table->name);
      exit(1);
    }

    current = current->next;
  }
}

SymbolTable *build_symbol_table(ASTNode *root) {
  if (!root) {
    return NULL;
  }

  SymbolTable *symbol_table = create_symbol_table();

  ASTNode *current = root;
  if (current->type == NODE_PROGRAM) {
    current = current->data.program.stmts;
  }

  NodeType currentType;
  char *currentName;
  while (current) {
    currentType = current->type;

    if (currentType == NODE_CREATE_STMT) {
      currentName = current->data.create_stmt.table_name;

      if (find_table(symbol_table, currentName)) {
        fprintf(stderr,
                "Semantic Error: Could not create table %s (table already "
                "exists).\n",
                currentName);
        exit(1);
      }

      Table *table = add_table(symbol_table, currentName);
      if (!table) {
        fprintf(stderr, "Semantic Error: Could not create table %s \n",
                currentName);
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
      fprintf(stderr, "Semantic Error: Could not find column %s at table %s \n",
              col_name, current_table->name);
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

  default:
    fprintf(stderr, "Semantic Error: Unknown expression node type.\n");
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

      if ((left != INT && left != FLOAT) || (right != INT && right != FLOAT)) {
        fprintf(stderr,
                "Semantic Error: Type Mismatch - left type(%s) op(%s) right "
                "type(%s) at line(%d)\n",
                column_type_to_string(left), op, column_type_to_string(right),
                expression->lineno);
        exit(1);
      }

      if (left == FLOAT || right == FLOAT) {
        return FLOAT;
      }
      return INT;
    } else if (strcmp(op, "%") == 0) {
      if (left != INT || right != INT) {
        fprintf(stderr,
                "Semantic Error: Modulo allows only INT - left(%s) op(%s) "
                "right(%s) at line(%d)\n",
                column_type_to_string(left), op, column_type_to_string(right),
                expression->lineno);
        exit(1);
      }
      return INT;
    } else {
      if (!type_checking(left, right)) {
        fprintf(stderr,
                "Semantic Error: Type Mismatch - left(%s) op(%s) right(%s) at "
                "line(%d)\n",
                column_type_to_string(left), op, column_type_to_string(right),
                expression->lineno);
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
        fprintf(
            stderr,
            "Semantic Error: Type Mismatch - op(%s) operand(%s) at line(%d)\n",
            op, column_type_to_string(operand), expression->lineno);
        exit(1);
      }
      return operand;
    } else if (strcmp(op, "!") == 0) {
      if (operand != BOOL) {
        fprintf(
            stderr,
            "Semantic Error: '!' allows only BOOL - operand(%s) at line(%d)\n",
            column_type_to_string(operand), expression->lineno);
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
              "Semantic Error: Type Mismatch - lhs(%s) rhs(%s) at line(%d)\n",
              column_type_to_string(lhs), column_type_to_string(rhs),
              expression->lineno);
      exit(1);
    }

    return lhs;
  }
  return NULL_TYPE;
}