#ifndef ANALYZER_H
#define ANALYZER_H

#include "ast/ast.h"
#include "model/symbol_table.h"

void run_semantics(ASTNode *root, SymbolTable *st);
void analyze_semantics(ASTNode *root, SymbolTable *st);

#endif