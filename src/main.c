#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "minish.h"
#include "parser.h"
#include "strutil.h"

static volatile sig_atomic_t sigchld_flag = 0;
static void on_sigchld(int signo) { (void)signo; sigchld_flag = 1; }

int main(void) {
    // Ignore Ctrl-C in the shell; children get default
    struct sigaction sa_int = {0};
    sa_int.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sa_int, NULL);

    // Reap finished children
    struct sigaction sa = {0};
    sa.sa_handler = on_sigchld;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    jobs_init();

    char *line = NULL;
    size_t cap = 0;
    for (;;) {
        if (sigchld_flag) { jobs_reap_zombies(); sigchld_flag = 0; }

        fputs("minish> ", stdout);
        fflush(stdout);

        ssize_t n = getline(&line, &cap, stdin);
        if (n < 0) { puts(""); break; }

        char *t = trim(line);
        if (!*t) continue;

        command_t *cmd = parse_line(t);
        if (!cmd) continue;

        (void)execute_pipeline(cmd, t);
        free_command(cmd);
    }

    free(line);
    jobs_free();
    return 0;
}
