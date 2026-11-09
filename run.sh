#!/bin/bash

echo "Cleaning old generated files..."
rm -f lex.yy.c y.tab.h y.tab.c y.output interpreter

echo "Generating Lexer and Parser..."
flex ./syntax_analyzer/Scanner.l
bison -d -y -v ./syntax_analyzer/Parser.y

echo "Compiling the Interpreter..."
gcc -I. -g -o interpreter \
    lex.yy.c \
    y.tab.c \
    ast/ast.c \
    model/symbol_table.c \
    semantic_analyzer/utils/semantic_utils.c \
    semantic_analyzer/analyzer.c \
    main.c

if [ $? -eq 0 ]; then
    echo "Build successful! Running Interpreter..."
    echo "========================================="
    
    ./interpreter input1.dbs input2.dbs input3.dbs
    
    echo "========================================="
    echo "Execution finished with status code $?"
else
    echo "Compilation failed!"
    exit 1
fi