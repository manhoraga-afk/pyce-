#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "builtins.h"
#include "platform.h"  // Platform-specific functions

// ... (existing builtins code from previous file)

Value builtin_split(Value text, Value delimiter) {
    if (text.type != VAL_TEXT || delimiter.type != VAL_TEXT ||
        !text.text || !delimiter.text || strlen(delimiter.text) == 0) {
        return make_group();
    }

    char* str = text.text;
    char* delim_str = delimiter.text;
    char delim = delim_str[0];  // Use first character only

    // Count tokens
    int count = 1;
    for (char* p = str; *p; p++) {
        if (*p == delim) count++;
    }

    Value group = make_group();
    group.group = malloc(count * sizeof(Value));
    group.group_count = 0;

    char* token = strtok(strdup(str), &delim);
    while (token && group.group_count < count) {
        group.group[group.group_count++] = make_text(token);
        token = strtok(NULL, &delim);
    }

    return group;
}

Value builtin_wait(Value arg) {
    if (arg.type == VAL_NUM) {
        // Wait for N seconds
        sleep((int)arg.num);
    } else if (arg.type == VAL_BOOL) {
        // Wait until boolean becomes true
        while (!arg.num) {
            sleep(1);
            // In real implementation: check the actual variable state
            // This is a simplified version
        }
    }
    return make_num(0);
}

Value builtin_ask_file() {
    return platform_ask_file();
}

Value builtin_read_file(Value path) {
    if (path.type != VAL_TEXT || !path.text) {
        return make_text("");
    }

    FILE* f = fopen(path.text, "r");
    if (!f) {
        return make_text("");
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';

    fclose(f);
    return make_text(content);
}

Value builtin_write_file(Value path, Value data) {
    if (path.type != VAL_TEXT || !path.text ||
        data.type != VAL_TEXT || !data.text) {
        return make_bool(0);
    }

    FILE* f = fopen(path.text, "w");
    if (!f) {
        return make_bool(0);
    }

    fwrite(data.text, 1, strlen(data.text), f);
    fclose(f);
    return make_bool(1);
}

Value builtin_refresh(Value screen_name) {
    // In real implementation: tell UI system to refresh this screen
    printf("Refreshing screen: %s\n", screen_name.text ? screen_name.text : "unknown");
    return make_num(0);
}

Value builtin_random(Value min, Value max) {
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }

    double min_val = min.type == VAL_NUM ? min.num : 0.0;
    double max_val = max.type == VAL_NUM ? max.num : 1.0;

    if (min_val > max_val) {
        double temp = min_val;
        min_val = max_val;
        max_val = temp;
    }

    double range = max_val - min_val;
    double rand_val = (double)rand() / RAND_MAX;
    return make_num(min_val + rand_val * range);
}

Value make_group() {
    Value v = {VAL_GROUP, 0, NULL};
    v.group = NULL;
    v.group_count = 0;
    return v;
}