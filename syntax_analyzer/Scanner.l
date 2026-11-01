%{

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

%}



%option yylineno 
%option noyywrap 
%option caseless

%x COMMENT


ALPHA      [a-zA-Z]
DIGIT      [0-9]
HEX        [0-9a-fA-F]


%%

"/%"            { BEGIN(COMMENT); } 
<COMMENT>.      {}  
<COMMENT>\n     {}
<COMMENT>"%/"   { BEGIN(INITIAL); }

[ \t\r\n]+      {  }



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


\"([^\"\n\\]|\\.)*\" { 
    int len = yyleng - 2;
    char* str = (char*)malloc(len + 1);
    
    int i = 1; 
    int j = 0;
    
    while (i <= len) {
        if (yytext[i] == '\\') {
            i++;
            switch(yytext[i]) {
                case 'n': str[j++] = '\n'; break;
                case 't': str[j++] = '\t'; break;
                case 'r': str[j++] = '\r'; break;
                case '\\': str[j++] = '\\'; break;
                case '\"': str[j++] = '\"'; break;
                default:  str[j++] = yytext[i]; break; 
            }
        } else {
            str[j++] = yytext[i];
        }
        i++;
    }
    str[j] = '\0'; 
    
    yylval.str_val = str;
    return STRING_LITERAL; 
}

\'([^'\n\\]|\\.)\'   { 
    if(yytext[1] != '\\') {
        yylval.char_val = yytext[1];
    } else {
        switch(yytext[2]){
            case 'n': yylval.char_val = '\n'; break;
            case 't': yylval.char_val = '\t'; break;
            case 'r': yylval.char_val = '\r'; break;
            case '\\': yylval.char_val = '\\'; break;
            case '\'': yylval.char_val = '\''; break;
            default:  yylval.char_val = yytext[2]; break; 
        }
    }
    return CHAR_LITERAL;
}

({ALPHA}|"_")({ALPHA}|{DIGIT}|"_")* {
    yylval.str_val = (char*)malloc(strlen(yytext) + 1);
    strcpy(yylval.str_val, yytext);
    return IDENTIFIER; 
}


"="                 { return EQ; }
"!="                { return NEQ; }
">="                { return GTE; }
"<="                { return LTE; }
"&&"                { return AND; }
"||"                { return OR; }



";"|","|"("|")"|"*"|">"|"<" {
    return yytext[0];
}


.                   {
    printf(""Lexical Error on line %d : Unrecognized character '%s'\n", yylineno, yytext);
    exit(1);
}

%%



int main() {
    yylex();
    return 0;
}