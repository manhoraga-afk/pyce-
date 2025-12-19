#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include "interpreter.h"
#include "vm.h"
#include "ast.h"

char* replace_ext(const char* filename, const char* new_ext) {
    static char buffer[1024];
    char* dot = strrchr(filename, '.');
    if (dot) {
        int len = dot - filename;
        if (len >= 1023) len = 1023;
        strncpy(buffer, filename, len);
        buffer[len] = '\0';
    } else {
        strncpy(buffer, filename, 1023);
        buffer[1023] = '\0';
    }
    strncat(buffer, new_ext, 1023 - strlen(buffer));
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Pycee++ v0.1\n");
        printf("A calm, powerful language that compiles to native binaries and runs interpreted\n\n");
        printf("Usage:\n");
        printf("  pycee run <file.pyc>     # Run in interpreted mode\n");
        printf("  pycee build <file.pyc>   # Compile to bytecode (.pycb)\n");
        printf("  pycee compile <file.pyc> # Compile to native binary\n");
        printf("\nExample:\n");
        printf("  bin/pycee run samples/hello.pyc\n");
        printf("  bin/pycee compile samples/hello.pyc && ./samples/hello\n");
        return 1;
    }

    char* cmd = argv[1];
    char* file = argv[2];

    if (access(file, F_OK) != 0) {
        fprintf(stderr, "Error: File not found: %s\n", file);
        return 1;
    }

    ASTNode* ast = parse_file(file);
    if (!ast) {
        fprintf(stderr, "Error: Failed to parse %s\n", file);
        return 1;
    }

    if (strcmp(cmd, "run") == 0) {
        printf("Running in interpreted mode...\n");
        interpret(ast);
    }
    else if (strcmp(cmd, "build") == 0) {
        char* output = replace_ext(file, ".pycb");
        printf("Compiling to bytecode...\n");
        compile_to_bytecode(ast, output);
    }
    else if (strcmp(cmd, "compile") == 0) {
        char* output_base = replace_ext(file, "");
        printf("Compiling to native binary...\n");
        compile_to_native(ast, output_base);
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        free_ast(ast);
        return 1;
    }

    free_ast(ast);
    return 0;
}