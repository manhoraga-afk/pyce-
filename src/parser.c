#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"

static Token* tokens;
static int current;
static int token_count;

static Token advance() {
    return tokens[current++];
}

static Token peek() {
    if (current >= token_count) {
        Token eof = {TOK_EOF, "", 0};
        return eof;
    }
    return tokens[current];
}

static Token previous() {
    return tokens[current - 1];
}

static int check(TokenType type) {
    if (current >= token_count) return 0;
    return peek().type == type;
}

static Token consume(TokenType type, const char* message) {
    if (check(type)) return advance();
    fprintf(stderr, "Parse error at line %d: %s\n", peek().line, message);
    exit(1);
}

static ASTNode* expression();
static ASTNode* statement();
static ASTNode* block();

static ASTNode* primary() {
    if (check(TOK_NUMBER)) {
        Token num = advance();
        ASTNode* node = new_node(NODE_LITERAL);
        strncpy(node->literal, num.lexeme, 255);
        return node;
    }
    if (check(TOK_IDENTIFIER)) {
        Token ident = advance();
        ASTNode* node = new_node(NODE_IDENTIFIER);
        strncpy(node->ident, ident.lexeme, 63);
        return node;
    }
    if (check(TOK_LPAREN)) {
        advance(); // consume '('
        ASTNode* expr = expression();
        consume(TOK_RPAREN, "Expect ')' after expression.");
        return expr;
    }
    fprintf(stderr, "Expect expression at line %d\n", peek().line);
    exit(1);
}

// ADD NEW FUNCTIONS FOR EXPRESSION PARSING:

static ASTNode* factor() {
    if (check(TOK_NUMBER)) {
        Token num = advance();
        ASTNode* node = new_node(NODE_LITERAL);
        strncpy(node->literal, num.lexeme, 255);
        return node;
    }
    if (check(TOK_IDENTIFIER)) {
        Token ident = advance();
        ASTNode* node = new_node(NODE_IDENTIFIER);
        strncpy(node->ident, ident.lexeme, 63);
        return node;
    }
    if (check(TOK_LPAREN)) {
        advance(); // consume '('
        ASTNode* expr = expression();
        consume(TOK_RPAREN, "Expect ')' after expression.");
        return expr;
    }
    if (check(TOK_NEGATIVE)) {
        advance(); // consume '-'
        ASTNode* num_node = factor();
        ASTNode* neg_node = new_node(NODE_UNARY_MINUS);
        neg_node->unary.operand = num_node;
        return neg_node;
    }
    fprintf(stderr, "Expect factor at line %d\n", peek().line);
    exit(1);
}

static ASTNode* term() {
    ASTNode* expr = factor();

    while (check(TOK_TIMES) || check(TOK_DIVIDE)) {
        Token op = advance();
        ASTNode* right = factor();

        ASTNode* node = new_node(NODE_BINARY);
        node->binary.left = expr;
        node->binary.right = right;
        node->binary.op = (op.type == TOK_TIMES) ? OP_TIMES : OP_DIVIDE;
        expr = node;
    }

    return expr;
}

static ASTNode* expression() {
    ASTNode* expr = term();

    while (check(TOK_PLUS) || check(TOK_MINUS)) {
        Token op = advance();
        ASTNode* right = term();

        ASTNode* node = new_node(NODE_BINARY);
        node->binary.left = expr;
        node->binary.right = right;
        node->binary.op = (op.type == TOK_PLUS) ? OP_PLUS : OP_MINUS;
        expr = node;
    }

    return expr;
}

static ASTNode* finish_call(ASTNode* callee) {
    ASTNode* args = NULL;
    ASTNode** tail = &args;
    int count = 0;

    if (!check(TOK_RPAREN)) {
        do {
            if (count >= 10) {
                fprintf(stderr, "Too many arguments\n");
                exit(1);
            }
            ASTNode* arg = expression();
            *tail = arg;
            tail = &((*tail)->next);
            count++;
        } while (check(TOK_COMMA) && (advance(), 1));
    }

    consume(TOK_RPAREN, "Expect ')' after arguments.");

    ASTNode* call_node = new_node(NODE_ACTION_CALL);
    strncpy(call_node->call.ident, callee->ident, 63);
    call_node->call.args = args;
    call_node->call.count = count;
    free(callee);
    return call_node;
}

static ASTNode* call() {
    ASTNode* expr = primary();

    while (1) {
        if (check(TOK_LPAREN)) {
            advance(); // consume '('
            expr = finish_call(expr);
        } else {
            break;
        }
    }

    return expr;
}

static ASTNode* expression() {
    return call();
}

static ASTNode* set_statement() {
    consume(TOK_SET, "Expect 'set' at start of assignment.");
    Token name = consume(TOK_IDENTIFIER, "Expect variable name.");
    consume(TOK_TO, "Expect 'to' after variable name.");
    ASTNode* value = expression();

    ASTNode* node = new_node(NODE_SET);
    strncpy(node->set.name, name.lexeme, 63);
    node->set.value = value;
    return node;
}

static ASTNode* report_statement() {
    consume(TOK_REPORT, "Expect 'report'.");
    ASTNode* expr = expression();

    ASTNode* node = new_node(NODE_REPORT);
    node->report.expr = expr;
    return node;
}

static ASTNode* if_statement() {
    consume(TOK_IF, "Expect 'if'.");
    ASTNode* condition = expression();
    consume(TOK_THEN, "Expect 'then' after condition.");

    ASTNode* then_block = block();
    ASTNode* else_block = NULL;

    if (check(TOK_ELSE)) {
        advance();
        else_block = block();
    }

    consume(TOK_END, "Expect 'end' after if statement.");

    ASTNode* node = new_node(NODE_IF);
    node->if_stmt.cond = condition;
    node->if_stmt.then_block = then_block;
    node->if_stmt.else_block = else_block;
    return node;
}

static ASTNode* loop_statement() {
    consume(TOK_LOOP, "Expect 'loop'.");
    ASTNode* condition = NULL;

    if (check(TOK_LPAREN)) {
        advance(); // consume '('
        if (!check(TOK_RPAREN)) {
            condition = expression();
        }
        consume(TOK_RPAREN, "Expect ')' after loop condition.");
    }

    ASTNode* body = block();
    consume(TOK_END, "Expect 'end' after loop body.");

    ASTNode* node = new_node(NODE_LOOP);
    node->loop.condition = condition;
    node->loop.body = body;
    return node;
}

static ASTNode* contain_statement() {
    consume(TOK_CONTAIN, "Expect 'contain'.");
    ASTNode* body = block();
    consume(TOK_END, "Expect 'end' after contain block.");

    ASTNode* node = new_node(NODE_CONTAIN);
    node->contain.body = body;
    return node;
}

static ASTNode* wait_statement() {
    consume(TOK_WAIT, "Expect 'wait'.");
    consume(TOK_LPAREN, "Expect '(' after wait.");
    ASTNode* condition = expression();
    consume(TOK_RPAREN, "Expect ')' after wait condition.");

    ASTNode* node = new_node(NODE_WAIT);
    node->wait.condition = condition;
    return node;
}

static ASTNode* exit_statement() {
    consume(TOK_EXIT, "Expect 'exit'.");
    ASTNode* node = new_node(NODE_EXIT);
    return node;
}

static ASTNode* skip_statement() {
    consume(TOK_SKIP, "Expect 'skip'.");
    ASTNode* node = new_node(NODE_SKIP);
    return node;
}

static ASTNode* block() {
    ASTNode* statements = NULL;
    ASTNode** tail = &statements;

    while (!check(TOK_END) && !check(TOK_ELSE) && !check(TOK_EOF)) {
        ASTNode* stmt = statement();
        if (!stmt) break;

        *tail = stmt;
        tail = &((*tail)->next);
    }

    return statements;
}

static ASTNode* statement() {
    if (check(TOK_SET)) return set_statement();
    if (check(TOK_REPORT)) return report_statement();
    if (check(TOK_IF)) return if_statement();
    if (check(TOK_LOOP)) return loop_statement();
    if (check(TOK_CONTAIN)) return contain_statement();
    if (check(TOK_WAIT)) return wait_statement();
    if (check(TOK_EXIT)) return exit_statement();
    if (check(TOK_SKIP)) return skip_statement();

    return expression();
}

ASTNode* parse_tokens(Token* tokens_ptr, int count) {
    tokens = tokens_ptr;
    token_count = count;
    current = 0;

    ASTNode* program = block();
    consume(TOK_EOF, "Expect end of file.");
    return program;
}

ASTNode* parse_file(const char* filename) {
    int count;
    Token* tokens = tokenize_file(filename, &count);
    ASTNode* ast = parse_tokens(tokens, count);
    free_tokens(tokens);
    return ast;
}