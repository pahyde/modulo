#include <stdlib.h>
#include <string.h>

#include "util.h"

char *format_time(time_t utc_time) {
    return "placeholder";
}

char *path_join(char *path1, char *path2, char separator) {
    size_t length1 = strlen(path1);
    size_t length2 = strlen(path2);
    char *joined = malloc(length1 + length2 + 2);
    for (size_t i = 0; i < length1; i++) {
        joined[i] = path1[i];
    }
    joined[length1] = separator;
    for (size_t i = 0; i < length2; i++) {
        joined[length1+1+i] = path2[i];
    }
    joined[length1 + length2 + 1] = '\0';
    return joined;
}