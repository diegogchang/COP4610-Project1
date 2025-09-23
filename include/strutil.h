#ifndef STRUTIL_H
#define STRUTIL_H
#include <stddef.h>

void *xcalloc(size_t n, size_t sz);
void *xmalloc(size_t sz);
char *xstrdup(const char *s);
char *trim(char *s);
char *strjoin_space(char *const *argv);

#endif
