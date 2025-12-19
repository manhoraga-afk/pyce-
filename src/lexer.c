#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const struct { const char* word; TokenType type; } keywords[] = {
    {"set", TOK_SET}, {"to", TOK_TO}, {"report", TOK_REPORT},
    {"join", TOK_JOIN}, {"tonumber", TOK_TONUMBER}, {"totext", TOK_TOTEXT},
    {"if", TOK_IF}, {"then", TOK_THEN}, {"else", TOK_ELSE}, {"end", TOK_END},
    {"loop", TOK_LOOP}, {"exit", TOK_EXIT}, {"skip", TOK_SKIP},
    {"action", TOK_ACTION}, {"do", TOK_DO}, {"contain", TOK_CONTAIN},
    {"wait", TOK_WAIT}, {"and", TOK_AND}, {"or", TOK_OR}, {"not", TOK_NOT}
};

static TokenType keyword_type(const char* word) {
    for (size_t i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i].word) == 0)
            return keywords[i].type;
    }
    return TOK_IDENTIFIER;
}

Token* tokenize_file(const char* filename, int* count) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    Token* tokens = malloc(sizeof(Token) * 1024);
    int token_count = 0;
    int line = 1;
    char* current = source;

    while (*current && token_count < 1024) {
        if (isspace(*current)) {
            if (*current == '\n') line++;
            current++;
            continue;
        }

        if (*current == '(') {
            tokens[token_count++] = (Token){TOK_LPAREN, "(", line};
            current++;
            continue;
        }
        if (*current == ')') {
            tokens[token_count++] = (Token){TOK_RPAREN, ")", line};
            current++;
            continue;
        }
        if (*current == ',') {
            tokens[token_count++] = (Token){TOK_COMMA, ",", line};
            current++;
            continue;
        }
        if (*current == '+') {
            tokens[token_count++] = (Token){TOK_PLUS, "+", line};
            current++;
            continue;
        }
        if (*current == '*') {
            tokens[token_count++] = (Token){TOK_TIMES, "*", line};
            current++;
            continue;
        }
        if (*current == '>') {
            tokens[token_count++] = (Token){TOK_GT, ">", line};
            current++;
            continue;
        }
        if (*current == '<') {
            tokens[token_count++] = (Token){TOK_LT, "<", line};
            current++;
            continue;
        }
        if (*current == '=') {
            tokens[token_count++] = (Token){TOK_EQ, "=", line};
            current++;
            continue;
        }

        if (isdigit(*current) || (*current == '-' && isdigit(current[1]))) {
            char num[256];
            char* end;
            strtod(current, &end);
            int len = end - current;
            if (len > 255) len = 255;
            strncpy(num, current, len);
            num[len] = '\0';
            tokens[token_count++] = (Token){TOK_NUMBER, num, line};
            current = end;
            continue;
        }

        if (isalpha(*current) || *current == '_') {
            char ident[256];
            int i = 0;
            while (isalnum(current[i]) || current[i] == '_') {
                ident[i] = current[i];
                i++;
            }
            ident[i] = '\0';
            current += i;
            tokens[token_count] = (Token){keyword_type(ident), "", line};
            strncpy(tokens[token_count].lexeme, ident, 255);
            token_count++;
            continue;
        }

        // ADD TO THE SYMBOL HANDLING SECTION:
       if (*current == '-') {
      // Check for negative number
       if (isdigit(current[1]) || (current[1] == '.' && isdigit(current[2]))) {
        tokens[token_count++] = (Token){TOK_NEGATIVE, "-", line};
        } else {
        tokens[token_count++] = (Token){TOK_MINUS, "-", line};
        }
        current++;
        continue;
        }

        if (*current == '/') {
         tokens[token_count++] = (Token){TOK_DIVIDE, "/", line};
         current++;
         continue;
         }

        fprintf(stderr, "Line %d: Unexpected character '%c'\n", line, *current);
        exit(1);
    }

    tokens[token_count++] = (Token){TOK_EOF, "", line};
    *count = token_count;
    free(source);
    return tokens;
}

void free_tokens(Token* tokens) {
    free(tokens);
}