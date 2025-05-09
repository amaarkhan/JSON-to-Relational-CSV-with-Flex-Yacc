%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.tab.h"
#include "ast.h"

int line_num = 1;
int col_num = 1;

// Helper to convert a \uXXXX sequence to UTF-8
char* decode_string(const char* text) {
    char* result = malloc(strlen(text) + 1); // Will grow if needed
    char* dst = result;
    const char* src = text;
    while (*src) {
        if (*src == '\\') {
            src++;
            if (*src == 'u') {
                src++;
                char hex[5] = {0};
                for (int i = 0; i < 4 && isxdigit(src[i]); i++) {
                    hex[i] = src[i];
                }
                if (strlen(hex) != 4) {
                    fprintf(stderr, "Error: Invalid Unicode escape at line %d, column %d\n", line_num, col_num);
                    exit(EXIT_FAILURE);
                }
                unsigned int code;
                sscanf(hex, "%x", &code);
                // Encode UTF-8 (basic 1–3 byte range)
                if (code <= 0x7F) {
                    *dst++ = (char)code;
                } else if (code <= 0x7FF) {
                    *dst++ = 0xC0 | ((code >> 6) & 0x1F);
                    *dst++ = 0x80 | (code & 0x3F);
                } else {
                    *dst++ = 0xE0 | ((code >> 12) & 0x0F);
                    *dst++ = 0x80 | ((code >> 6) & 0x3F);
                    *dst++ = 0x80 | (code & 0x3F);
                }
                src += 4;
            } else {
                // Handle other escapes like \n, \t, etc.
                switch (*src) {
                    case 'n': *dst++ = '\n'; break;
                    case 't': *dst++ = '\t'; break;
                    case '"': *dst++ = '"'; break;
                    case '\\': *dst++ = '\\'; break;
                    default:
                        fprintf(stderr, "Error: Unknown escape sequence \\%c at line %d, column %d\n", *src, line_num, col_num);
                        exit(EXIT_FAILURE);
                }
                src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    return result;
}
%}

%option noyywrap

%%

[ \t]+         { col_num += yyleng; } // Skip whitespace
\n             { line_num++; col_num = 1; }

"{"            { col_num += yyleng; return LBRACE; }
"}"            { col_num += yyleng; return RBRACE; }
"["            { col_num += yyleng; return LBRACKET; }
"]"            { col_num += yyleng; return RBRACKET; }
":"            { col_num += yyleng; return COLON; }
","            { col_num += yyleng; return COMMA; }

\"([^"\\]|\\u[0-9a-fA-F]{4}|\\[nt"\\])*\" {
    yytext[yyleng - 1] = '\0'; // remove trailing quote
    yylval.str_val = decode_string(yytext + 1); // decode inside quotes
    col_num += yyleng;
    return STRING;
}

-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
    yylval.num_val = atof(yytext);
    col_num += yyleng;
    return NUMBER;
}

"true"         { yylval.boolean = 1; col_num += yyleng; return TRUE; }
"false"        { yylval.boolean = 0; col_num += yyleng; return FALSE; }
"null"         { col_num += yyleng; return NULLVAL; }

.              {
    fprintf(stderr, "Error: Unexpected character '%s' at line %d, column %d\n", yytext, line_num, col_num);
    exit(EXIT_FAILURE);
}

%%

