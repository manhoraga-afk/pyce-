#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parser.h"
#include "builtins.h"
#include "ast.h"

#define MAX_VARS 100
#define MAX_STACK 256

typedef struct {
    char name[64];
    Value value;
} Variable;

static Variable variables[MAX_VARS];
static int var_count = 0;

static Value get_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0)
            return variables[i].value;
    }
    return make_text(name);
}

static void set_variable(const char* name, Value value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            free_value(&variables[i].value);
            variables[i].value = value;
            return;
        }
    }
    if (var_count >= MAX_VARS) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    strncpy(variables[var_count].name, name, 63);
    variables[var_count].value = value;
    var_count++;
}

static Value evaluate(ASTNode* node) {

    if (!node) return make_num(0);

    switch (node->type) {
        case NODE_LITERAL:
            if (strchr(node->literal, '.') || strchr(node->literal, 'e') || strchr(node->literal, 'E')) {
                return make_num(atof(node->literal));
            } else {
                return make_num(atoi(node->literal));
            }

        case NODE_IDENTIFIER:
            return get_variable(node->ident);

        case NODE_ACTION_CALL:
            if (strcmp(node->call.ident, "join") == 0) {
                Value args[10];
                int count = node->call.count;
                ASTNode* arg_node = node->call.args;
                for (int i = 0; i < count && i < 10; i++) {
                    args[i] = evaluate(arg_node);
                    arg_node = arg_node->next;
                }
                return builtin_join(args, count);
            }
            else if (strcmp(node->call.ident, "tonumber") == 0) {
                ASTNode* arg = node->call.args;
                Value val = evaluate(arg);
                return builtin_tonumber(val);
            }
            else if (strcmp(node->call.ident, "totext") == 0) {
                ASTNode* arg = node->call.args;
                Value val = evaluate(arg);
                return builtin_totext(val);
            }
            else if (strcmp(node->call.ident, "random") == 0) {
                ASTNode* a = node->call.args;
                ASTNode* b = a ? a->next : NULL;
                int min = a ? (int)evaluate(a).num : 0;
                int max = b ? (int)evaluate(b).num : 1;
                if (min > max) {
                    int temp = min;
                    min = max;
                    max = temp;
                }
                return make_num(min + rand() % (max - min + 1));
            }
            fprintf(stderr, "Unknown function: %s\n", node->call.ident);
            exit(1);

        case NODE_SET:
            {
                Value val = evaluate(node->set.value);
                set_variable(node->set.name, val);
                return val;
            }

        case NODE_REPORT:
            {
                Value val = evaluate(node->report.expr);
                print_value(val);
                printf("\n");
                free_value(&val);
                return make_num(0);
            }

        case NODE_IF:
            {
                Value cond = evaluate(node->if_stmt.cond);
                int is_true = (cond.type == VAL_NUM && cond.num != 0) ||
                             (cond.type == VAL_BOOL && cond.num != 0) ||
                             (cond.type == VAL_TEXT && cond.text && strlen(cond.text) > 0);
                free_value(&cond);

                if (is_true) {
                    return evaluate(node->if_stmt.then_block);
                } else if (node->if_stmt.else_block) {
                    return evaluate(node->if_stmt.else_block);
                }
                return make_num(0);
            }

        case NODE_LOOP:
            {
                srand(time(NULL));

                if (!node->loop.condition) {
                    // Infinite loop
                    while (1) {
                        Value result = evaluate(node->loop.body);
                        // Check for exit/skip signals later
                        free_value(&result);
                    }
                } else {
                    // Condition-based loop
                    while (1) {
                        Value cond = evaluate(node->loop.condition);
                        int is_true = (cond.type == VAL_NUM && cond.num != 0) ||
                                     (cond.type == VAL_BOOL && cond.num != 0) ||
                                     (cond.type == VAL_TEXT && cond.text && strlen(cond.text) > 0);
                        free_value(&cond);

                        if (!is_true) break;

                        Value result = evaluate(node->loop.body);
                        // Check for exit/skip signals later
                        free_value(&result);
                    }
                }
                return make_num(0);
            }

        case NODE_EXIT:
            // In real implementation, this would set a flag to break loops
            printf("Exit called\n");
            exit(0);

        case NODE_CONTAIN:
            // In real implementation, this would spawn a thread
            printf("Contain block (background task):\n");
            return evaluate(node->contain.body);

        case NODE_WAIT:
            {
                Value cond = evaluate(node->wait.condition);
                if (cond.type == VAL_NUM) {
                    // Wait for N seconds
                    printf("Waiting for %.0f seconds...\n", cond.num);
                    sleep((int)cond.num);
                } else {
                    // Wait for condition to be true
                    printf("Waiting for condition...\n");
                    // In real implementation, this would loop and check
                    sleep(1);
                }
                free_value(&cond);
                return make_num(0);
            }

        case NODE_BLOCK:
            {
                ASTNode* stmt = node;
                Value last = make_num(0);
                while (stmt) {
                    free_value(&last);
                    last = evaluate(stmt);
                    stmt = stmt->next;
                }
                return last;
            }

        default:
            fprintf(stderr, "Unexpected node type: %d\n", node->type);
            exit(1);
    }
}



void interpret(ASTNode* ast) {
    srand(time(NULL));
    evaluate(ast);
}

