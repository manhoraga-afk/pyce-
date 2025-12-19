#ifndef VM_H
#define VM_H

void compile_to_bytecode(ASTNode* ast, const char* output_path);
void vm_run(const char* bytecode_path);

#endif