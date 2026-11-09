#include "ast/ast.h"
#include "semantic_analyzer/analyzer.h"
#include "semantic_analyzer/utils/semantic_utils.h"
#include <stdio.h>
#include <stdlib.h>

extern FILE *yyin;
extern int yyparse(void);
extern ASTNode *root;
extern int yylineno;

const char *current_filename = "<stdin>";

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file1.dbs> [file2.dbs ...]\n", argv[0]);
    return 1;
  }

  FILE *out_file = fopen("output.txt", "w");
  if (!out_file) {
    fprintf(stderr, "Error: Could not create output.txt\n");
    return 1;
  }

  SymbolTable *symbol_table = create_symbol_table();

  for (int i = 1; i < argc; i++) {
    current_filename = argv[i];
    FILE *file = fopen(current_filename, "r");

    if (!file) {
      fprintf(stderr, "Error: Could not open file '%s'\n", current_filename);
      fclose(out_file);
      free_symbol_table(symbol_table);
      return 1;
    }

    yyin = file;
    yylineno = 1;
    root = NULL;

    if (yyparse() != 0) {
      fclose(file);
      fclose(out_file);
      free_symbol_table(symbol_table);
      return 1;
    }

    run_semantics(root, symbol_table);

    fprintf(out_file, "=== AST FOR FILE: %s ===\n", current_filename);
    print_ast(root, out_file, 0);
    fprintf(out_file, "\n");

    free_ast(root);
    fclose(file);
  }

  fclose(out_file);
  free_symbol_table(symbol_table);

  return 0;
}