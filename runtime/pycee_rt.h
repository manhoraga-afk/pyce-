#ifndef PYCEE_RT_H
#define PYCEE_RT_H

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

Value pyc_join(Value* args, int count);
Value pyc_tonumber(Value arg);
Value pyc_totext(Value arg);
Value pyc_random(double min, double max);
int pyc_is_true(Value v);
void pyc_print(Value v);

#endif