#include "ast.h"

extern int lineno;

ASTNode *root = NULL;

ASTNode *create_int_node(int val, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }
  node->type = NODE_INT_LITERAL;
  node->lineno = lineno;
  node->data.int_val = val;

  node->next = NULL;

  return node;
}

ASTNode *create_real_node(double val, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_REAL_LITERAL;
  node->lineno = lineno;
  node->data.real_val = val;

  node->next = NULL;

  return node;
}

ASTNode *create_string_node(char *val, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_STRING_LITERAL;
  node->lineno = lineno;

  node->data.str_val = (char *)malloc(strlen(val) + 1);
  if (!node->data.str_val) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  strcpy(node->data.str_val, val);

  node->next = NULL;

  return node;
}

ASTNode *create_bool_node(int val, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_BOOL_LITERAL;
  node->lineno = lineno;
  node->data.int_val = val;

  node->next = NULL;

  return node;
}

ASTNode *create_null_node(int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_NULL_LITERAL;
  node->lineno = lineno;

  node->next = NULL;

  return node;
}

ASTNode *create_identifier_node(char *name, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_IDENTIFIER;
  node->lineno = lineno;

  node->data.str_val = (char *)malloc(strlen(name) + 1);
  if (!node->data.str_val) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  strcpy(node->data.str_val, name);

  node->next = NULL;

  return node;
}

ASTNode *create_binary_expr_node(char *op, ASTNode *left, ASTNode *right,
                                 int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_BINARY_EXPR;
  node->lineno = lineno;

  BinaryExpr binaryExpr = {op ? strdup(op) : NULL, left, right};
  node->data.binary = binaryExpr;

  node->next = NULL;

  return node;
}

ASTNode *create_unary_expr_node(char *op, ASTNode *operand, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_UNARY_EXPR;
  node->lineno = lineno;

  UnaryExpr unaryExpr = {op ? strdup(op) : NULL, operand};
  node->data.unary = unaryExpr;

  node->next = NULL;

  return node;
}

ASTNode *create_func_call_node(char *name, ASTNode *arg, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_FUNC_CALL;
  node->lineno = lineno;

  FuncCall funcCall = {name ? strdup(name) : NULL, arg};
  node->data.func = funcCall;

  node->next = NULL;

  return node;
}

ASTNode *create_select_node(int is_distinct, ASTNode *select_list,
                            char *table_name, ASTNode *where_clause,
                            ASTNode *group_by, ASTNode *having,
                            ASTNode *order_by, int limit_val, int lineno) {

  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_SELECT_STMT;
  node->lineno = lineno;

  SelectStmt selectStmt = {
      is_distinct,  select_list, table_name ? strdup(table_name) : NULL,
      where_clause, group_by,    having,
      order_by,     limit_val};
  node->data.select_stmt = selectStmt;

  node->next = NULL;

  return node;
}

ASTNode *create_create_table_node(char *table_name, ASTNode *column_list,
                                  int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_CREATE_STMT;
  node->lineno = lineno;

  CreateStmt createStmt = {table_name ? strdup(table_name) : NULL, column_list};
  node->data.create_stmt = createStmt;

  node->next = NULL;

  return node;
}

ASTNode *create_insert_table_node(char *table_name, ASTNode *column_list,
                                  ASTNode *value_list, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_INSERT_STMT;
  node->lineno = lineno;

  InsertStmt insertStmt = {table_name ? strdup(table_name) : NULL, column_list,
                           value_list};
  node->data.insert_stmt = insertStmt;

  node->next = NULL;

  return node;
}

ASTNode *create_update_table_node(char *table_name, ASTNode *update_list,
                                  ASTNode *where_clause, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_UPDATE_STMT;
  node->lineno = lineno;

  UpdateStmt updateStmt = {table_name ? strdup(table_name) : NULL, update_list,
                           where_clause};
  node->data.update_stmt = updateStmt;

  node->next = NULL;

  return node;
}

ASTNode *create_delete_table_node(char *table_name, ASTNode *where_clause,
                                  int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_DELETE_STMT;
  node->lineno = lineno;

  DeleteStmt deleteStmt = {table_name ? strdup(table_name) : NULL,
                           where_clause};
  node->data.delete_stmt = deleteStmt;

  node->next = NULL;

  return node;
}

ASTNode *create_drop_table_node(char *table_name, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error: Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_DROP_STMT;
  node->lineno = lineno;
  node->data.drop_stmt.table_name = table_name ? strdup(table_name) : NULL;
  node->next = NULL;

  return node;
}

ASTNode *create_column_def_node(char *name, char *type, int is_pk, int is_nn,
                                int lineno) {

  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_COLUMN_DEF;
  node->lineno = lineno;

  ColumnDef columnDef = {name ? strdup(name) : NULL, type ? strdup(type) : NULL,
                         is_pk, is_nn};
  node->data.col_def = columnDef;

  node->next = NULL;

  return node;
}

ASTNode *create_assignment_node(ASTNode *identifier, ASTNode *expr,
                                int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_ASSIGNMENT;
  node->lineno = lineno;

  Assignment assignment = {identifier, expr};
  node->data.assignment = assignment;

  node->next = NULL;

  return node;
}

ASTNode *create_order_by_item_node(ASTNode *expr, int is_desc, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_ORDER_BY_ITEM;
  node->lineno = lineno;

  OrderByItem orderByItem = {expr, is_desc};
  node->data.order_by_item = orderByItem;

  node->next = NULL;

  return node;
}

ASTNode *create_program_node(ASTNode *stmts, int lineno) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) {
    fprintf(stderr, "Error : Out of memory at line %d\n", lineno);
    exit(1);
  }

  node->type = NODE_PROGRAM;
  node->lineno = lineno;

  ProgramNode programNode = {stmts};
  node->data.program = programNode;

  node->next = NULL;

  return node;
}

static void print_indent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");
  }
}

void print_ast(ASTNode *node, int indent) {
  if (!node)
    return;

  print_indent(indent);

  switch (node->type) {
  case NODE_INT_LITERAL:
    printf("[INT] %d\n", node->data.int_val);
    break;

  case NODE_REAL_LITERAL:
    printf("[REAL] %f\n", node->data.real_val);
    break;

  case NODE_STRING_LITERAL:
    printf("[STRING] \"%s\"\n", node->data.str_val);
    break;

  case NODE_BOOL_LITERAL:
    printf("[BOOL] %s\n", node->data.int_val ? "TRUE" : "FALSE");
    break;

  case NODE_NULL_LITERAL:
    printf("[NULL]\n");
    break;

  case NODE_IDENTIFIER:
    printf("[IDENTIFIER] %s\n", node->data.str_val);
    break;

  case NODE_BINARY_EXPR:
    printf("[BINARY EXPR] Op: %s\n", node->data.binary.op);
    print_ast(node->data.binary.left, indent + 1);
    print_ast(node->data.binary.right, indent + 1);
    break;

  case NODE_UNARY_EXPR:
    printf("[UNARY EXPR] Op: %s\n", node->data.unary.op);
    print_ast(node->data.unary.operand, indent + 1);
    break;

  case NODE_ASSIGNMENT:
    printf("[ASSIGNMENT]\n");
    print_ast(node->data.assignment.identifier, indent + 1);
    print_ast(node->data.assignment.expr, indent + 1);
    break;

  case NODE_ORDER_BY_ITEM:
    printf("[ORDER BY ITEM] %s\n",
           node->data.order_by_item.is_desc ? "DESC" : "ASC");
    print_ast(node->data.order_by_item.expr, indent + 1);
    break;

  case NODE_SELECT_STMT:
    printf("[SELECT STMT] Table: %s, Distinct: %d, Limit: %d\n",
           node->data.select_stmt.from_table ? node->data.select_stmt.from_table
                                             : "NULL",
           node->data.select_stmt.is_distinct,
           node->data.select_stmt.limit_val);

    if (node->data.select_stmt.select_list) {
      print_indent(indent + 1);
      printf("Select List:\n");
      print_ast(node->data.select_stmt.select_list, indent + 2);
    }
    if (node->data.select_stmt.where_clause) {
      print_indent(indent + 1);
      printf("WHERE:\n");
      print_ast(node->data.select_stmt.where_clause, indent + 2);
    }
    if (node->data.select_stmt.order_by) {
      print_indent(indent + 1);
      printf("ORDER BY:\n");
      print_ast(node->data.select_stmt.order_by, indent + 2);
    }
    break;

  case NODE_CREATE_STMT:
    printf("[CREATE TABLE STMT] Table: %s\n",
           node->data.create_stmt.table_name);
    print_ast(node->data.create_stmt.column_list, indent + 1);
    break;

  case NODE_COLUMN_DEF:
    printf("[COLUMN DEF] Name: %s, Type: %s (PK: %d, NOT NULL: %d)\n",
           node->data.col_def.name, node->data.col_def.data_type,
           node->data.col_def.is_primary_key, node->data.col_def.is_not_null);
    break;

  case NODE_INSERT_STMT:
    printf("[INSERT STMT] Table: %s\n", node->data.insert_stmt.table_name);
    if (node->data.insert_stmt.value_list) {
      print_indent(indent + 1);
      printf("Values:\n");
      print_ast(node->data.insert_stmt.value_list, indent + 2);
    }
    break;

  case NODE_UPDATE_STMT:
    printf("[UPDATE STMT] Table: %s\n", node->data.update_stmt.table_name);
    print_indent(indent + 1);
    printf("SET:\n");
    print_ast(node->data.update_stmt.update_list, indent + 2);
    if (node->data.update_stmt.where_clause) {
      print_indent(indent + 1);
      printf("WHERE:\n");
      print_ast(node->data.update_stmt.where_clause, indent + 2);
    }
    break;

  case NODE_DELETE_STMT:
    printf("[DELETE STMT] Table: %s\n", node->data.delete_stmt.table_name);
    if (node->data.delete_stmt.where_clause) {
      print_indent(indent + 1);
      printf("WHERE:\n");
      print_ast(node->data.delete_stmt.where_clause, indent + 2);
    }
    break;

  case NODE_DROP_STMT:
    printf("[DROP TABLE STMT] Table: %s\n", node->data.drop_stmt.table_name);
    break;

  case NODE_FUNC_CALL:
    printf("[FUNC CALL] Name: %s\n",
           node->data.func.name ? node->data.func.name : "NULL");
    if (node->data.func.arg) {
      print_ast(node->data.func.arg, indent + 1);
    }
    break;

  default:
    printf("[UNKNOWN NODE TYPE: %d]\n", node->type);
    break;
  }

  if (node->next) {
    print_ast(node->next, indent);
  }
}

ASTNode *append_ast_node(ASTNode *head, ASTNode *new_node) {
  if (!head)
    return new_node;
  if (!new_node)
    return head;

  ASTNode *curr = head;
  while (curr->next != NULL) {
    curr = curr->next;
  }
  curr->next = new_node;
  return head;
}

void free_ast(ASTNode *node) {
  if (!node)
    return;

  if (node->next) {
    free_ast(node->next);
    node->next = NULL;
  }

  switch (node->type) {
  case NODE_IDENTIFIER:
  case NODE_STRING_LITERAL:
    if (node->data.str_val)
      free(node->data.str_val);
    break;

  case NODE_BINARY_EXPR:
    if (node->data.binary.op)
      free(node->data.binary.op);
    free_ast(node->data.binary.left);
    free_ast(node->data.binary.right);
    break;

  case NODE_UNARY_EXPR:
    if (node->data.unary.op)
      free(node->data.unary.op);
    free_ast(node->data.unary.operand);
    break;

  case NODE_FUNC_CALL:
    if (node->data.func.name)
      free(node->data.func.name);
    free_ast(node->data.func.arg);
    break;

  case NODE_COLUMN_DEF:
    if (node->data.col_def.name)
      free(node->data.col_def.name);
    if (node->data.col_def.data_type)
      free(node->data.col_def.data_type);
    break;
  case NODE_ASSIGNMENT:
    free_ast(node->data.assignment.identifier);
    free_ast(node->data.assignment.expr);
    break;
  case NODE_ORDER_BY_ITEM:
    free_ast(node->data.order_by_item.expr);
    break;

  case NODE_SELECT_STMT:
    free_ast(node->data.select_stmt.select_list);
    if (node->data.select_stmt.from_table)
      free(node->data.select_stmt.from_table);
    free_ast(node->data.select_stmt.where_clause);
    free_ast(node->data.select_stmt.group_by);
    free_ast(node->data.select_stmt.having);
    free_ast(node->data.select_stmt.order_by);
    break;

  case NODE_CREATE_STMT:
    if (node->data.create_stmt.table_name)
      free(node->data.create_stmt.table_name);
    free_ast(node->data.create_stmt.column_list);
    break;

  case NODE_INSERT_STMT:
    if (node->data.insert_stmt.table_name)
      free(node->data.insert_stmt.table_name);
    free_ast(node->data.insert_stmt.column_list);
    free_ast(node->data.insert_stmt.value_list);
    break;

  case NODE_UPDATE_STMT:
    if (node->data.update_stmt.table_name)
      free(node->data.update_stmt.table_name);
    free_ast(node->data.update_stmt.update_list);
    free_ast(node->data.update_stmt.where_clause);
    break;

  case NODE_DELETE_STMT:
    if (node->data.delete_stmt.table_name)
      free(node->data.delete_stmt.table_name);
    free_ast(node->data.delete_stmt.where_clause);
    break;

  case NODE_DROP_STMT:
    if (node->data.drop_stmt.table_name)
      free(node->data.drop_stmt.table_name);
    break;

  default:
    break;
  }

  free(node);
}