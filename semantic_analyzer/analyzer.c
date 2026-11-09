#include "analyzer.h"
#include "utils/semantic_utils.h"
#include <stdio.h>
#include <stdlib.h>

extern const char *current_filename;

void run_semantics(ASTNode *root, SymbolTable *st) {
  if (!root || !st) {
    return;
  }

  build_symbol_table(root, st);

  analyze_semantics(root, st);
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
      fprintf(stderr,
              "Semantic Error in '%s' at line %d: Unknown statement type.\n",
              current_filename, current->lineno);
      exit(1);
    }

    current = current->next;
  }
}