cmake_minimum_required(VERSION 3.10)
project(DBInterpreter C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(SQLParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)

FLEX_TARGET(SQLLexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

ADD_FLEX_BISON_DEPENDENCY(SQLLexer SQLParser)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}       
    ${CMAKE_CURRENT_BINARY_DIR}       
)

add_executable(db_interpreter
    main.c
    ast/ast.c
    ${BISON_SQLParser_OUTPUTS}
    ${FLEX_SQLLexer_OUTPUTS}
)