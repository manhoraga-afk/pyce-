#ifndef BUILTINS_H
#define BUILTINS_H

typedef enum {
    VAL_NUM,
    VAL_TEXT,
    VAL_BOOL
} ValType;

typedef struct {
    ValType type;
    double num;
    char* text;
} Value;

Value make_num(double n);
Value make_text(const char* s);
Value make_bool(int b);
void free_value(Value* v);
void print_value(Value v);

Value builtin_join(Value* args, int count);
Value builtin_tonumber(Value arg);
Value builtin_totext(Value arg);

#endif