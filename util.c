#include "util.h"
#include <stdio.h>

void flushBuffer() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

size_t safeInput() {
    size_t i;
    scanf("%d", &i);
    flushBuffer();
    return i;
}