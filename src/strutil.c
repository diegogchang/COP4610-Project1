#include "strutil.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xcalloc(size_t n, size_t sz) { void *p = calloc(n, sz); if (!p) { perror("calloc"); exit(1);} return p; }
void *xmalloc(size_t sz)           { void *p = malloc(sz);     if (!p) { perror("malloc"); exit(1);} return p; }

char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = malloc(n + 1);
    if (!p) { perror("malloc"); exit(1); }
    memcpy(p, s, n + 1);
    return p;
}

char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return s;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
    return s;
}

char *strjoin_space(char *const *argv) {
    size_t len = 0;
    for (size_t i = 0; argv && argv[i]; ++i) len += strlen(argv[i]) + 1;
    if (!len) return xstrdup("");
    char *out = xmalloc(len);
    out[0] = '\0';
    for (size_t i = 0; argv[i]; ++i) {
        strcat(out, argv[i]);
        if (argv[i + 1]) strcat(out, " ");
    }
    return out;
}
