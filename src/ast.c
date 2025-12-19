#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* new_node(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(1);
    }
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_SET:
            free_ast(node->set.value);
            break;
        case NODE_REPORT:
            free_ast(node->report.expr);
            break;
        case NODE_IF:
            free_ast(node->if_stmt.cond);
            free_ast(node->if_stmt.then_block);
            free_ast(node->if_stmt.else_block);
            break;
        case NODE_LOOP:
            free_ast(node->loop.condition);
            free_ast(node->loop.body);
            break;
        case NODE_CONTAIN:
            free_ast(node->contain.body);
            break;
        case NODE_WAIT:
            free_ast(node->wait.condition);
            break;
        case NODE_ACTION_CALL:
            {
                ASTNode* arg = node->call.args;
                while (arg) {
                    ASTNode* next = arg->next;
                    free_ast(arg);
                    arg = next;
                }
            }
            break;
        default:
            break;
    }

    free_ast(node->next);
    free(node);
}