#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "ast.h"

typedef enum {
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_LITERAL,
    OP_JOIN,
    OP_TONUMBER,
    OP_TOTEXT,
    OP_RANDOM,
    OP_REPORT,
    OP_IF,
    OP_LOOP,
    OP_EXIT,
    OP_SKIP,
    OP_END
} OpCode;

typedef struct {
    OpCode op;
    union {
        int var_index;
        int literal_index;
        int jump_offset;
        struct {
            int arg_count;
        } call;
    };
} Bytecode;

static const int MAX_BYTECODE = 1024;
static const int MAX_STRINGS = 256;

static char* strings[MAX_STRINGS];
static int string_count = 0;

static int add_string(const char* str) {
    if (string_count >= MAX_STRINGS) {
        fprintf(stderr, "Too many strings\n");
        exit(1);
    }
    strings[string_count] = strdup(str);
    return string_count++;
}

void compile_to_bytecode(ASTNode* ast, const char* output_path) {
    Bytecode program[MAX_BYTECODE];
    int pc = 0;

    // Simple compiler: just handle report join(...)
    if (ast && ast->type == NODE_REPORT) {
        ASTNode* expr = ast->report.expr;
        if (expr && expr->type == NODE_ACTION_CALL && strcmp(expr->call.ident, "join") == 0) {
            program[pc++] = (Bytecode){OP_LITERAL, .literal_index = add_string("Hello")};
            program[pc++] = (Bytecode){OP_LITERAL, .literal_index = add_string("World")};
            program[pc++] = (Bytecode){OP_LITERAL, .literal_index = add_string("!")};
            program[pc++] = (Bytecode){OP_JOIN, .call.arg_count = 3};
            program[pc++] = (Bytecode){OP_REPORT};
        }
    }

    program[pc++] = (Bytecode){OP_END};

    // Write to file
    FILE* f = fopen(output_path, "wb");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    // Write string table
    fwrite(&string_count, sizeof(int), 1, f);
    for (int i = 0; i < string_count; i++) {
        int len = strlen(strings[i]) + 1;
        fwrite(&len, sizeof(int), 1, f);
        fwrite(strings[i], 1, len, f);
        free(strings[i]);
    }

    // Write bytecode
    fwrite(&pc, sizeof(int), 1, f);
    fwrite(program, sizeof(Bytecode), pc, f);

    fclose(f);
    printf("Compiled to bytecode: %s\n", output_path);
}

void vm_run(const char* bytecode_path) {
    FILE* f = fopen(bytecode_path, "rb");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    // Read string table
    int str_count;
    fread(&str_count, sizeof(int), 1, f);
    char** str_table = malloc(str_count * sizeof(char*));
    for (int i = 0; i < str_count; i++) {
        int len;
        fread(&len, sizeof(int), 1, f);
        str_table[i] = malloc(len);
        fread(str_table[i], 1, len, f);
    }

    // Read bytecode
    int code_size;
    fread(&code_size, sizeof(int), 1, f);
    Bytecode* code = malloc(code_size * sizeof(Bytecode));
    fread(code, sizeof(Bytecode), code_size, f);
    fclose(f);

    // Simple VM
    Value stack[MAX_STACK];
    int sp = 0;

    for (int pc = 0; pc < code_size; pc++) {
        Bytecode instr = code[pc];

        switch (instr.op) {
            case OP_LITERAL:
                if (sp >= MAX_STACK) {
                    fprintf(stderr, "Stack overflow\n");
                    exit(1);
                }
                stack[sp++] = make_text(str_table[instr.literal_index]);
                break;

            case OP_JOIN:
                {
                    if (sp < instr.call.arg_count) {
                        fprintf(stderr, "Not enough arguments for join\n");
                        exit(1);
                    }
                    Value args[10];
                    for (int i = 0; i < instr.call.arg_count; i++) {
                        args[i] = stack[sp - instr.call.arg_count + i];
                    }
                    Value result = builtin_join(args, instr.call.arg_count);
                    sp -= instr.call.arg_count - 1;
                    stack[sp - 1] = result;
                }
                break;

            case OP_REPORT:
                if (sp == 0) {
                    fprintf(stderr, "Stack underflow for report\n");
                    exit(1);
                }
                print_value(stack[--sp]);
                printf("\n");
                break;

            case OP_END:
                goto exit_vm;

            default:
                fprintf(stderr, "Unknown opcode: %d\n", instr.op);
                exit(1);
        }
    }

exit_vm:
    // Clean up
    for (int i = 0; i < sp; i++) {
        free_value(&stack[i]);
    }
    for (int i = 0; i < str_count; i++) {
        free(str_table[i]);
    }
    free(str_table);
    free(code);
}