# DB-Compiler

A lightweight Frontend Compiler for a custom SQL-like database query language. 
Built using C, Flex (Lexer), and Bison (Parser).

## Architecture & Pipeline
1. **Lexical Analysis (Flex):** Tokenizes incoming raw SQL-like string inputs.
2. **Syntax Analysis (Bison):** Parses tokens according to custom grammar rules.
3. **AST Generation:** Constructs an Abstract Syntax Tree (AST) representing the query.

## Supported Commands (Initial Scope)
- `CREATE TABLE <table_name> (<col1> <type>, ...);`
- `INSERT INTO <table_name> VALUES (<val1>, ...);`
- `SELECT <cols> FROM <table_name> [WHERE <condition>];`

## Building
```bash
make
./db_compiler