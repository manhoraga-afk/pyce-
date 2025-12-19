#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static FILE* out_file;
static int indent_level = 0;

static void indent() {
    for (int i = 0; i < indent_level; i++) {
        fprintf(out_file, "  ");
    }
}

static void compile_node(ASTNode* node);

static void compile_expression(ASTNode* node) {
    switch (node->type) {
        case NODE_LITERAL:
            fprintf(out_file, "\"%s\"", node->literal);
            break;
        case NODE_IDENTIFIER:
            fprintf(out_file, "%s", node->ident);
            break;
        case NODE_ACTION_CALL:
            if (strcmp(node->call.ident, "join") == 0) {
                fprintf(out_file, "pyc_join((Value[]){");
                ASTNode* arg = node->call.args;
                for (int i = 0; i < node->call.count; i++) {
                    if (i > 0) fprintf(out_file, ", ");
                    compile_expression(arg);
                    arg = arg->next;
                }
                fprintf(out_file, "}, %d)", node->call.count);
            } else if (strcmp(node->call.ident, "tonumber") == 0) {
                fprintf(out_file, "pyc_tonumber(");
                compile_expression(node->call.args);
                fprintf(out_file, ")");
            } else if (strcmp(node->call.ident, "totext") == 0) {
                fprintf(out_file, "pyc_totext(");
                compile_expression(node->call.args);
                fprintf(out_file, ")");
            } else if (strcmp(node->call.ident, "random") == 0) {
                fprintf(out_file, "pyc_random(");
                if (node->call.args) {
                    compile_expression(node->call.args);
                    if (node->call.args->next) {
                        fprintf(out_file, ", ");
                        compile_expression(node->call.args->next);
                    } else {
                        fprintf(out_file, ", 1");
                    }
                } else {
                    fprintf(out_file, "0, 1");
                }
                fprintf(out_file, ")");
            } else {
                fprintf(stderr, "Unknown function: %s\n", node->call.ident);
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Unexpected node in expression: %d\n", node->type);
            exit(1);
    }
}

static void compile_statement(ASTNode* node) {
    switch (node->type) {
        case NODE_SET:
            indent();
            fprintf(out_file, "Value %s = ", node->set.name);
            compile_expression(node->set.value);
            fprintf(out_file, ";\n");
            break;
        case NODE_REPORT:
            indent();
            fprintf(out_file, "pyc_print(");
            compile_expression(node->report.expr);
            fprintf(out_file, "); printf(\"\\n\");\n");
            break;
        case NODE_IF:
            indent();
            fprintf(out_file, "if (pyc_is_true(");
            compile_expression(node->if_stmt.cond);
            fprintf(out_file, ")) {\n");
            indent_level++;
            compile_node(node->if_stmt.then_block);
            indent_level--;
            indent();
            fprintf(out_file, "} else {\n");
            indent_level++;
            if (node->if_stmt.else_block) {
                compile_node(node->if_stmt.else_block);
            } else {
                indent();
                fprintf(out_file, "// empty else block\n");
            }
            indent_level--;
            indent();
            fprintf(out_file, "}\n");
            break;
        default:
            fprintf(stderr, "Unexpected node in statement: %d\n", node->type);
            exit(1);
    }
}

static void compile_node(ASTNode* node) {
    while (node) {
        compile_statement(node);
        node = node->next;
    }
}

void compile_to_native(ASTNode* ast, const char* output_base) {
    char c_file[512];
    char bin_file[512];

    snprintf(c_file, sizeof(c_file), "%s.c", output_base);
    snprintf(bin_file, sizeof(bin_file), "%s", output_base);

    out_file = fopen(c_file, "w");
    if (!out_file) {
        perror("fopen");
        exit(1);
    }

    // Write C header
    fprintf(out_file, "#include \"pycee_rt.h\"\n\n");
    fprintf(out_file, "int main() {\n");
    indent_level = 1;

    // Compile AST
    compile_node(ast);

    // Write footer
    indent();
    fprintf(out_file, "return 0;\n");
    fprintf(out_file, "}\n");
    fclose(out_file);

    // Compile C to binary
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "gcc -o %s %s.c runtime/pycee_rt.c -Iinclude -lm",
             bin_file, output_base);

    printf("Compiling: %s\n", cmd);
    int result = system(cmd);
    if (result != 0) {
        fprintf(stderr, "Compilation failed\n");
        exit(1);
    }

    printf("Compiled to native binary: %s\n", bin_file);
}