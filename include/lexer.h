#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOK_SET, TOK_TO, TOK_REPORT, TOK_JOIN, TOK_TONUMBER, TOK_TOTEXT,
    TOK_IF, TOK_THEN, TOK_ELSE, TOK_END, TOK_LOOP, TOK_EXIT, TOK_SKIP,
    TOK_ACTION, TOK_DO, TOK_CONTAIN, TOK_WAIT,
    TOK_IDENTIFIER, TOK_NUMBER, TOK_LPAREN, TOK_RPAREN,
    TOK_COMMA, TOK_PLUS, TOK_TIMES, TOK_GT, TOK_LT, TOK_EQ,
    TOK_NOT, TOK_AND, TOK_OR, TOK_EOF,TOK_MINUS, TOK_DIVIDE, TOK_NEGATIVE
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[256];
    int line;
} Token;

Token* tokenize_file(const char* filename, int* count);
void free_tokens(Token* tokens);

#endif