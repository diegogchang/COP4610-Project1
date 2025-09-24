#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exec.h"
#include "minish.h"
#include "parser.h"
#include "strutil.h"

#define MAX_HISTORY 3

static volatile sig_atomic_t sigchld_flag = 0;
static void on_sigchld(int signo) { (void)signo; sigchld_flag = 1; }

// Command history for exit builtin
static char *command_history[MAX_HISTORY];
static int history_count = 0;

static void add_to_history(const char *cmd) {
    // Free the oldest entry if we're at capacity
    if (history_count == MAX_HISTORY) {
        free(command_history[0]);
        // Shift entries left
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            command_history[i] = command_history[i + 1];
        }
        history_count--;
    }
    
    // Add new command
    command_history[history_count] = xstrdup(cmd);
    history_count++;
}

void print_command_history(void) {
    if (history_count == 0) {
        printf("No valid commands executed.\n");
        return;
    }
    
    printf("Last %s valid command%s:\n", 
           history_count == 1 ? "" : (history_count <= 3 ? "few" : "three"),
           history_count == 1 ? "" : "s");
    
    for (int i = 0; i < history_count; i++) {
        printf("  %s\n", command_history[i]);
    }
}

static void cleanup_history(void) {
    for (int i = 0; i < history_count; i++) {
        free(command_history[i]);
        command_history[i] = NULL;
    }
    history_count = 0;
}

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

        printf("%s@%s:%s> ", 
            getenv("USER") ?: "user",
            getenv("HOSTNAME") ?: "localhost", 
            getenv("PWD") ?: getcwd(NULL, 0));
        fflush(stdout);

        ssize_t n = getline(&line, &cap, stdin);
        if (n < 0) { puts(""); break; }

        char *t = trim(line);
        if (!*t) continue;

        command_t *cmd = parse_line(t);
        if (!cmd) continue;

        // Add valid command to history (but not if it's just "exit")
        if (cmd->argv && cmd->argv[0] && strcmp(cmd->argv[0], "exit") != 0) {
            add_to_history(t);
        }

        (void)execute_pipeline(cmd, t);
        free_command(cmd);
    }

    free(line);
    cleanup_history();
    jobs_free();
    return 0;
}