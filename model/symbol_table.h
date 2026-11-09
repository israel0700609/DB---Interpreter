#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <stdio.h>
#include <string.h>

typedef enum ColumnType { INT, FLOAT, BOOL, TEXT, NULL_TYPE } ColumnType;

typedef struct Column {
  char *name;
  ColumnType type;
  int is_primary_key;
  int is_not_null;
  struct Column *next;
} Column;

typedef struct Table {
  char *name;
  Column *columns;
  struct Table *next;
} Table;

typedef struct SymbolTable {
  Table *tables;
} SymbolTable;

SymbolTable *create_symbol_table();

Table *add_table(SymbolTable *st, char *name);

void drop_table(SymbolTable *st, char *name);

int add_column_to_table(Table *table, char *name, ColumnType type, int is_pk,
                        int is_nn);
Table *find_table(SymbolTable *st, char *name);

int add_column_to_table(Table *table, char *name, ColumnType type, int is_pk,
                        int is_nn);

Column *find_column(Table *table, char *name);

ColumnType string_to_column_type(char *type_str);

void print_symbol_table(SymbolTable *st, FILE *out);

void free_symbol_table(SymbolTable *st);

#endif
