#ifndef STRING_H
#define STRING_H

#include <stdint.h>

int  str_eq(const char *a, const char *b);
int  str_starts_with(const char *str, const char *prefix);
void str_copy(char *dst, const char *src, int n);

#endif