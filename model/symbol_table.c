#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable *create_symbol_table() {
  SymbolTable *symbol_table = (SymbolTable *)malloc(sizeof(SymbolTable));
  if (!symbol_table) {
    fprintf(stderr, "Error: Out of memory\n");
    exit(1);
  }
  symbol_table->tables = NULL;
  return symbol_table;
}

Table *find_table(SymbolTable *st, char *name) {
  if (st == NULL || name == NULL) {
    return NULL;
  }
  Table *current = st->tables;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

Table *add_table(SymbolTable *st, char *name) {
  if (!st || !name)
    return NULL;

  if (find_table(st, name) != NULL) {
    return NULL;
  }

  Table *new_table = (Table *)malloc(sizeof(Table));
  if (!new_table) {
    fprintf(stderr, "Error: Out of memory\n");
    exit(1);
  }
  new_table->name = strdup(name);
  new_table->columns = NULL;
  new_table->next = NULL;

  if (st->tables == NULL) {
    st->tables = new_table;
  } else {
    Table *current = st->tables;
    while (current->next) {
      current = current->next;
    }
    current->next = new_table;
  }

  return new_table;
}

void free_single_table(Table *table) {
  if (!table)
    return;

  Column *curr_col = table->columns;
  while (curr_col) {
    Column *next_col = curr_col->next;
    if (curr_col->name)
      free(curr_col->name);
    free(curr_col);
    curr_col = next_col;
  }

  if (table->name)
    free(table->name);
  free(table);
}

void drop_table(SymbolTable *st, char *name) {
  if (!st || !st->tables) {
    fprintf(stderr, "Semantic Error: Table '%s' does not exist to drop.\n",
            name);
    exit(1);
  }

  if (strcmp(st->tables->name, name) == 0) {
    Table *to_delete = st->tables;
    st->tables = st->tables->next;

    free_single_table(to_delete);
    return;
  }

  Table *current = st->tables;
  while (current->next) {
    if (strcmp(current->next->name, name) == 0) {
      Table *to_delete = current->next;

      current->next = to_delete->next;

      free_single_table(to_delete);
      return;
    }
    current = current->next;
  }

  fprintf(stderr, "Semantic Error: Table '%s' not found to drop.\n", name);
  exit(1);
}

Column *find_column(Table *table, char *name) {
  if (!table || !name) {
    return NULL;
  }

  Column *current = table->columns;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

int add_column_to_table(Table *table, char *name, ColumnType type, int is_pk,
                        int is_nn) {
  if (!table || !name) {
    return 0;
  }
  if (find_column(table, name) != NULL) {
    return 0;
  }

  Column *new_column = (Column *)malloc(sizeof(Column));
  if (!new_column) {
    fprintf(stderr, "Error: Out of memory\n");
    exit(1);
  }
  new_column->name = strdup(name);
  new_column->type = type;
  new_column->is_primary_key = is_pk;
  new_column->is_not_null = is_nn;
  new_column->next = NULL;

  if (table->columns == NULL) {
    table->columns = new_column;
  } else {
    Column *current = table->columns;
    while (current->next) {
      current = current->next;
    }
    current->next = new_column;
  }

  return 1;
}

ColumnType string_to_column_type(char *type_str) {
  if (type_str == NULL) {
    return NULL_TYPE;
  }

  if (strcmp(type_str, "INT") == 0) {
    return INT;
  } else if (strcmp(type_str, "FLOAT") == 0) {
    return FLOAT;
  } else if (strcmp(type_str, "BOOL") == 0) {
    return BOOL;
  } else if (strcmp(type_str, "TEXT") == 0) {
    return TEXT;
  } else {
    return NULL_TYPE;
  }
}

void print_symbol_table(SymbolTable *st, FILE *out) {
  if (!st) {
    fprintf(out, "Symbol Table is NULL\n");
    return;
  }

  fprintf(out, "===== SYMBOL TABLE =====\n");
  Table *curr_table = st->tables;

  while (curr_table) {
    fprintf(out, "Table: %s\n", curr_table->name);
    Column *curr_col = curr_table->columns;

    while (curr_col) {
      char *type_str = "UNKNOWN";
      switch (curr_col->type) {
      case INT:
        type_str = "INT";
        break;
      case FLOAT:
        type_str = "FLOAT";
        break;
      case BOOL:
        type_str = "BOOL";
        break;
      case TEXT:
        type_str = "TEXT";
        break;
      default:
        type_str = "NULL";
        break;
      }

      fprintf(out, "  -> Column: %s | Type: %s", curr_col->name, type_str);
      if (curr_col->is_primary_key)
        fprintf(out, " [PRIMARY KEY]");
      if (curr_col->is_not_null)
        fprintf(out, " [NOT NULL]");
      fprintf(out, "\n");

      curr_col = curr_col->next;
    }
    curr_table = curr_table->next;
  }
  fprintf(out, "========================\n");
}

void free_symbol_table(SymbolTable *st) {
  if (!st)
    return;

  Table *curr_table = st->tables;
  while (curr_table) {
    Table *next_table = curr_table->next;

    Column *curr_col = curr_table->columns;
    while (curr_col) {
      Column *next_col = curr_col->next;
      if (curr_col->name)
        free(curr_col->name);
      free(curr_col);
      curr_col = next_col;
    }

    if (curr_table->name)
      free(curr_table->name);
    free(curr_table);

    curr_table = next_table;
  }

  free(st);
}