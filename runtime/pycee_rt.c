#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "pycee_rt.h"

static Value make_num(double n) {
    Value v = {VAL_NUM, n, NULL};
    return v;
}

static Value make_text(const char* s) {
    Value v = {VAL_TEXT, 0, NULL};
    if (s) {
        v.text = malloc(strlen(s) + 1);
        if (!v.text) {
            perror("malloc");
            exit(1);
        }
        strcpy(v.text, s);
    }
    return v;
}

static Value make_bool(int b) {
    Value v = {VAL_BOOL, b ? 1.0 : 0.0, NULL};
    return v;
}

void free_value(Value* v) {
    if (v->type == VAL_TEXT && v->text) {
        free(v->text);
        v->text = NULL;
    }
}

Value pyc_join(Value* args, int count) {
    int all_numbers = 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type != VAL_NUM) {
            all_numbers = 0;
            break;
        }
    }

    if (all_numbers) {
        char result[512] = "";
        for (int i = 0; i < count; i++) {
            char num[64];
            if (args[i].num == (long)args[i].num)
                sprintf(num, "%.0f", args[i].num);
            else
                sprintf(num, "%g", args[i].num);
            strncat(result, num, sizeof(result) - strlen(result) - 1);
        }
        return make_text(result);
    } else {
        char result[512] = "";
        for (int i = 0; i < count; i++) {
            if (i > 0) strncat(result, " ", sizeof(result) - strlen(result) - 1);
            if (args[i].type == VAL_NUM) {
                char num[64];
                if (args[i].num == (long)args[i].num)
                    sprintf(num, "%.0f", args[i].num);
                else
                    sprintf(num, "%g", args[i].num);
                strncat(result, num, sizeof(result) - strlen(result) - 1);
            } else if (args[i].type == VAL_TEXT && args[i].text) {
                strncat(result, args[i].text, sizeof(result) - strlen(result) - 1);
            } else {
                strncat(result, args[i].num ? "yes" : "no", sizeof(result) - strlen(result) - 1);
            }
        }
        return make_text(result);
    }
}

Value pyc_tonumber(Value arg) {
    if (arg.type == VAL_NUM) return arg;

    if (arg.type == VAL_TEXT && arg.text) {
        char* end;
        double num = strtod(arg.text, &end);
        if (*end == '\0') return make_num(num);

        long sum = 0;
        for (char* p = arg.text; *p; p++) {
            sum += (unsigned char)*p;
        }
        return make_num((double)sum);
    }

    char buf[256];
    if (arg.type == VAL_BOOL) {
        strcpy(buf, arg.num ? "yes" : "no");
    } else {
        sprintf(buf, "%g", arg.num);
    }

    long sum = 0;
    for (char* p = buf; *p; p++) {
        sum += (unsigned char)*p;
    }
    return make_num((double)sum);
}

Value pyc_totext(Value arg) {
    char buf[256];
    if (arg.type == VAL_NUM) {
        if (arg.num == (long)arg.num)
            sprintf(buf, "%.0f", arg.num);
        else
            sprintf(buf, "%g", arg.num);
    } else if (arg.type == VAL_BOOL) {
        strcpy(buf, arg.num ? "yes" : "no");
    } else {
        return arg;
    }
    return make_text(buf);
}

Value pyc_random(double min, double max) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    double range = max - min;
    double rand_val = (double)rand() / RAND_MAX;
    return make_num(min + rand_val * range);
}

int pyc_is_true(Value v) {
    if (v.type == VAL_NUM) return v.num != 0;
    if (v.type == VAL_BOOL) return v.num != 0;
    if (v.type == VAL_TEXT) return v.text && strlen(v.text) > 0;
    return 0;
}

void pyc_print(Value v) {
    if (v.type == VAL_NUM) {
        if (v.num == (long)v.num)
            printf("%.0f", v.num);
        else
            printf("%g", v.num);
    } else if (v.type == VAL_TEXT) {
        printf("%s", v.text ? v.text : "");
    } else {
        printf("%s", v.num ? "yes" : "no");
    }
}