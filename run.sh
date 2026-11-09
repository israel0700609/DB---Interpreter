#!/bin/bash
echo "Deleting Files"
rm -f lex.yy.c
rm -f y.tab.h
rm -f y.tab.c
echo "Generating lex and yacc"
lex ./syntax_analyzer/Scanner.l
bison -d -y -v ./syntax_analyzer/Parser.y
echo "Compiling the Interpreter..."
gcc lex.yy.c y.tab.c ast/ast.c model/symbol_table.c semantic_analyzer/utils/semantic_utils.c semantic_analyzer/analyzer.c -I. -g -o interpreter

./interpreter output.txt < input.txt >> output.txt
echo "Interpreter Built and Executed"