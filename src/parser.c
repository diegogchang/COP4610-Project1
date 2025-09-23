#include "parser.h"
#include "strutil.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *dup_range(const char *s, size_t a, size_t b) {
    if (b < a) b = a;
    size_t n = b - a;
    char *p = malloc(n + 1);
    if (!p) { perror("malloc"); exit(1); }
    memcpy(p, s + a, n);
    p[n] = '\0';
    return p;
}

static void push_token(char ***arr, size_t *n, const char *tok) {
    *arr = realloc(*arr, (*n + 2) * sizeof(char *));
    if (!*arr) { perror("realloc"); exit(1); }
    (*arr)[(*n)++] = xstrdup(tok);
    (*arr)[*n] = NULL;
}

/* very simple ${VAR} expansion inside tokens */
static void expand_env_inplace(char *s) {
    char buf[4096]; buf[0] = '\0';
    size_t i = 0;

    while (s[i] && strlen(buf) < sizeof(buf) - 2) {
        if (s[i] == '$' && s[i + 1] == '{') {
            size_t j = i + 2;
            while (s[j] && s[j] != '}') j++;
            if (s[j] == '}') {
                char name[256];
                size_t len = j - (i + 2);
                if (len >= sizeof(name)) len = sizeof(name) - 1;
                memcpy(name, s + i + 2, len);
                name[len] = '\0';
                const char *val = getenv(name);
                if (!val) val = "";
                strncat(buf, val, sizeof(buf) - strlen(buf) - 1);
                i = j + 1;
                continue;
            }
        }
        char c[2] = { s[i], 0 };
        strncat(buf, c, sizeof(buf) - strlen(buf) - 1);
        i++;
    }
    strcpy(s, buf);
}

static command_t *make_cmd(void) {
    command_t *c = calloc(1, sizeof(command_t));
    if (!c) { perror("calloc"); exit(1); }
    return c;
}

command_t *parse_line(const char *line) {
    char *s = xstrdup(line);
    size_t i = 0, n = strlen(s);

    command_t *head = make_cmd();
    command_t *cur  = head;

    char **tokens = NULL; size_t ntok = 0;

    while (i < n) {
        while (i < n && isspace((unsigned char)s[i])) i++;
        if (i >= n) break;

        if (s[i] == '|') {
            if (ntok == 0 && !cur->infile && !cur->outfile) {
                fprintf(stderr, "syntax error near '|'\n");
                free(s); free_command(head); return NULL;
            }
            cur->argv = tokens; tokens = NULL; ntok = 0;
            cur->next_pipe = make_cmd();
            cur = cur->next_pipe;
            i++; continue;
        }

        if (s[i] == '<' || s[i] == '>') {
            int out = (s[i] == '>');
            int append = 0;
            if (out && i + 1 < n && s[i + 1] == '>') { append = 1; i += 2; }
            else { i++; }
            while (i < n && isspace((unsigned char)s[i])) i++;
            if (i >= n) { fprintf(stderr, "syntax error: expected filename after redirection\n"); free(s); free_command(head); return NULL; }
            size_t a = i;
            int inq = 0; char q = 0;
            while (i < n && (inq || (!isspace((unsigned char)s[i]) && s[i] != '|' && s[i] != '&'))) {
                if ((s[i] == '\'' || s[i] == '"')) {
                    if (!inq) { inq = 1; q = s[i++]; continue; }
                    else if (q == s[i]) { inq = 0; i++; continue; }
                }
                i++;
            }
            char *fname = dup_range(s, a, i);
            expand_env_inplace(fname);
            if (out) { cur->outfile = fname; cur->append = append; }
            else     { cur->infile  = fname; }
            continue;
        }

        if (s[i] == '&') {
            cur->background = 1;
            i++;
            while (i < n && isspace((unsigned char)s[i])) i++;
            break; // ignore trailing after &
        }

        int inq = 0; char q = 0;
        size_t a = i;
        while (i < n && (inq ||
                         (!isspace((unsigned char)s[i]) && s[i] != '|' && s[i] != '<' && s[i] != '>' && s[i] != '&'))) {
            if ((s[i] == '\'' || s[i] == '"')) {
                if (!inq) { inq = 1; q = s[i++]; continue; }
                else if (q == s[i]) { inq = 0; i++; continue; }
            }
            i++;
        }
        char *tok = dup_range(s, a, i);
        expand_env_inplace(tok);
        push_token(&tokens, &ntok, tok);
        free(tok);
    }

    cur->argv = tokens;
    if (!head->argv && !head->infile && !head->outfile) { free_command(head); head = NULL; }
    free(s);
    return head;
}

void free_command(command_t *cmd) {
    while (cmd) {
        command_t *next = cmd->next_pipe;
        if (cmd->argv) {
            for (size_t i = 0; cmd->argv[i]; ++i) free(cmd->argv[i]);
            free(cmd->argv);
        }
        free(cmd->infile);
        free(cmd->outfile);
        free(cmd);
        cmd = next;
    }
}
