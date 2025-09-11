#define _GNU_SOURCE
#include "exec.h"
#include "builtins.h"
#include "minish.h"
#include "strutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <glob.h>

static int setup_redirs(command_t *c){
    if (c->infile){
        int fd = open(c->infile, O_RDONLY);
        if (fd < 0){ perror("open <"); return -1; }
        if (dup2(fd, STDIN_FILENO) < 0){ perror("dup2 <"); close(fd); return -1; }
        close(fd);
    }
    if (c->outfile){
        int flags = O_WRONLY | O_CREAT | (c->append ? O_APPEND : O_TRUNC);
        int fd = open(c->outfile, flags, 0644);
        if (fd < 0){ perror("open >"); return -1; }
        if (dup2(fd, STDOUT_FILENO) < 0){ perror("dup2 >"); close(fd); return -1; }
        close(fd);
    }
    return 0;
}

static char **expand_globs(char **argv){
    glob_t g;
    size_t outcap = 16, outn = 0;
    char **out = malloc(outcap * sizeof(char*));
    if (!out){ perror("malloc"); exit(1); }
    for (size_t i = 0; argv[i]; ++i){
        memset(&g, 0, sizeof(g));
        int r = glob(argv[i], GLOB_TILDE, NULL, &g);
        if (r == 0){
            for (size_t k = 0; k < g.gl_pathc; ++k){
                if (outn + 2 >= outcap){
                    outcap *= 2;
                    out = realloc(out, outcap * sizeof(char*));
                    if (!out){ perror("realloc"); exit(1); }
                }
                out[outn++] = xstrdup(g.gl_pathv[k]);
            }
            globfree(&g);
        } else {
            if (outn + 2 >= outcap){
                outcap *= 2;
                out = realloc(out, outcap * sizeof(char*));
                if (!out){ perror("realloc"); exit(1); }
            }
            out[outn++] = xstrdup(argv[i]);
        }
    }
    out[outn] = NULL;
    return out;
}

static void child_exec(command_t *c, int in_fd, int out_fd, pid_t pgid){
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigaction(SIGINT, &sa, NULL);

    if (pgid == 0) pgid = getpid();
    setpgid(0, pgid);

    if (in_fd  != STDIN_FILENO){ dup2(in_fd,  STDIN_FILENO);  close(in_fd);  }
    if (out_fd != STDOUT_FILENO){ dup2(out_fd, STDOUT_FILENO); close(out_fd); }
    if (setup_redirs(c) < 0) _exit(127);

    if (!c->argv || !c->argv[0]) _exit(0);

    if (is_builtin(c->argv[0])){
        _exit(run_builtin(c->argv));
    } else {
        char **expanded = expand_globs(c->argv);
        execvp(expanded[0], expanded);
        perror("execvp");
        _exit(127);
    }
}

int execute_pipeline(command_t *cmd, const char *orig_line){
    if (cmd && !cmd->next_pipe && !cmd->infile && !cmd->outfile && !cmd->background
        && cmd->argv && cmd->argv[0] && is_builtin(cmd->argv[0])){
        return run_builtin(cmd->argv);
    }

    int in_fd = STDIN_FILENO;
    int pipefd[2];
    pid_t pgid = 0;
    command_t *c = cmd;

    while (c){
        int out_fd = STDOUT_FILENO;
        if (c->next_pipe){
            if (pipe(pipefd) < 0){ perror("pipe"); return 1; }
            out_fd = pipefd[1];
        }

        pid_t pid = fork();
        if (pid < 0){ perror("fork"); return 1; }

        if (pid == 0){
            if (c->next_pipe) close(pipefd[0]);
            child_exec(c, in_fd, out_fd, pgid);
        } else {
            if (pgid == 0) pgid = pid;
            setpgid(pid, pgid);
            if (in_fd != STDIN_FILENO) close(in_fd);
            if (c->next_pipe){
                close(out_fd);
                in_fd = pipefd[0];
            }
        }
        c = c->next_pipe;
    }

    if (cmd->background){
        jobs_add(pgid, orig_line);
        printf("[bg] started pgid=%d\n", (int)pgid);
        return 0;
    } else {
        int status = 0;
        pid_t w;
        do { w = waitpid(-pgid, &status, 0); }
        while (w > 0 || (w < 0 && errno == EINTR));
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
}
