#ifndef UI_COMPILER_H
#define UI_COMPILER_H

#include "ast.h"

typedef enum {
    TARGET_WINDOWS,
    TARGET_MACOS,
    TARGET_IOS,
    TARGET_ANDROID,
    TARGET_LINUX,
    TARGET_WEB
} TargetPlatform;

void compile_ui_to_platform(ASTNode* ui_ast, TargetPlatform target, const char* output_dir);
void compile_all_platforms(ASTNode* ui_ast, const char* base_output_dir);

#endif