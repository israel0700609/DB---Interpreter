%{

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

%}


    /* options */
%option yylineno 
%option noyywrap 
%option caseless

%x COMMENT


ALPHA      [a-zA-Z]
DIGIT      [0-9]
HEX        [0-9a-fA-F]


%%

"/%"            { BEGIN(COMMENT); } 
<COMMENT>.      { /* consume regular chars */}  
<COMMENT>\n     { /* consume new line char */}
<COMMENT>"%/"   { BEGIN(INITIAL); }

/* consume space, tab and new line */
[ \t\r\n]+      {  }


    /* keywords */

"CREATE"        { return CREATE; }
"DROP"          { return DROP; }
"TABLE"         { return TABLE; }
"PRIMARY"       { return PRIMARY; }
"KEY"           { return KEY; }
"NOT"           { return NOT; }
"NULL"          { return NULL_TOK; }


"INT"           { return TYPE_INT; }
"INTEGER"       { return TYPE_INT; }
"TEXT"          { return TYPE_TEXT; }
"FLOAT"         { return TYPE_FLOAT; }
"BOOLEAN"       { return TYPE_BOOL; }


"SELECT"        { return SELECT; }
"INSERT"        { return INSERT; }
"INTO"          { return INTO; }
"VALUES"        { return VALUES; }
"UPDATE"        { return UPDATE; }
"SET"           { return SET; }
"DELETE"        { return DELETE; }
"FROM"          { return FROM; }
"WHERE"         { return WHERE; }


"AND"           { return AND; }
"OR"            { return OR; }


"GROUP"         { return GROUP; }
"BY"            { return BY; }
"HAVING"        { return HAVING; }
"ORDER"         { return ORDER; }
"ASC"           { return ASC; }
"DESC"          { return DESC; }
"LIMIT"         { return LIMIT; }
"DISTINCT"      { return DISTINCT; }

"UNION"         { return UNION; }
"INTERSECT"     { return INTERSECT; }
"MINUS"         { return MINUS; }
"EXISTS"        { return EXISTS; }

"IN"            { return IN; }
"IS"            { return IS; }

    /* aggression functions */
"COUNT"         { return COUNT; }
"SUM"           { return SUM; }
"AVG"           { return AVG; }
"MIN"           { return MIN; }
"MAX"           { return MAX; }


    /* literals */
"TRUE"          { 
    yylval.int_val = 1; 
    return TRUE_LITERAL;
}
"FALSE"         { 
    yylval.int_val = 0;    
    return FALSE_LITERAL; 
}

0|[1-9]{DIGIT}*   {
    yylval.int_val = atoi(yytext);
    return INT_LITERAL;
}

0[xX]{HEX}+        {
    yylval.int_val = strtol(yytext + 2, NULL, 16);
    return INT_LITERAL;
}

([0-9]+\.[0-9]*|\.[0-9]+)([eE][-+]?[0-9]+)?|[0-9]+[eE][-+]?[0-9]+ { 
    yylval.real_val = atof(yytext);    
    return REAL_LITERAL; 
}


\'([^'\n\\]|\\.)*\' { 
    int len = yyleng - 2; 
    char* str = (char*)malloc(len + 1);
    
    int j = 0;
    for (int i = 1; i <= len; ) {
        if (yytext[i] == '\\') {
            i++; 
            switch (yytext[i]) {
                case 'n':  str[j++] = '\n'; break;
                case 't':  str[j++] = '\t'; break;
                case 'r':  str[j++] = '\r'; break;
                case '\\': str[j++] = '\\'; break;
                case '\'': str[j++] = '\''; break;
                default:   
                    str[j++] = '\\';
                    str[j++] = yytext[i]; 
                    break; 
            }
            i++;
        } else {
            str[j++] = yytext[i++];
        }
    }
    str[j] = '\0';
    
    yylval.str_val = str;
    return STRING_LITERAL; 
}


    /* identifier */
({ALPHA}|"_")({ALPHA}|{DIGIT}|"_")* {
    yylval.str_val = (char*)malloc(strlen(yytext) + 1);
    strcpy(yylval.str_val, yytext);
    return IDENTIFIER; 
}


"="                 { return EQ; }
"!="                { return NEQ; }
">="                { return GTE; }
"<="                { return LTE; }


"+"|"-"|"/"|"%"|";"|","|"("|")"|"*"|">"|"<" {
    return yytext[0];
}

.                   {
    printf("Lexical Error on line %d : Unrecognized character '%s'\n", yylineno, yytext);
    exit(1);
}

%%



int main() {

    yylex();
    return 0;
}