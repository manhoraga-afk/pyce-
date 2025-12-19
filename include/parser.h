#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

ASTNode* parse_tokens(Token* tokens, int count);
ASTNode* parse_file(const char* filename);

#endif