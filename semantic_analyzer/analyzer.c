#include "analyzer.h"
#include "utils/semantic_utils.h"

void run_semantics(ASTNode *root) {
  if (!root) {
    fprintf(stderr, "AST root must be defiend.\n");
    exit(1);
  }

  SymbolTable *symbol_table = build_symbol_table(root);
  if (!symbol_table) {
    fprintf(stderr, "Semantic Error: Could not build symbol table.\n");
    exit(1);
  }

  analyze_semantics(root, symbol_table);
}

void analyze_semantics(ASTNode *root, SymbolTable *st) {
  if (!root) {
    return;
  }

  ASTNode *current = root;

  if (current->type == NODE_PROGRAM) {
    current = current->data.program.stmts;
  }

  while (current) {
    switch (current->type) {
    case NODE_CREATE_STMT:
      break;

    case NODE_SELECT_STMT:
      check_select_semantics(current, st);
      break;

    case NODE_INSERT_STMT:
      check_insert_semantics(current, st);
      break;

    case NODE_UPDATE_STMT:
      check_update_semantics(current, st);
      break;

    case NODE_DELETE_STMT:
      check_delete_semantics(current, st);
      break;

    case NODE_DROP_STMT:
      check_drop_statement(current, st);
      break;

    default:
      fprintf(stderr, "Semantic Error: Unknown statement type at line %d.\n",
              current->lineno);
      exit(1);
    }

    current = current->next;
  }
}
