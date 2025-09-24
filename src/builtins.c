#include "builtins.h"
#include "strutil.h"
#include "minish.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// External function from main.c to print command history
extern void print_command_history(void);

static int bi_cd(char **argv) { 
    const char *d = argv[1] ? argv[1] : getenv("HOME"); 
    if (!d) { 
        fprintf(stderr, "cd: HOME not set\n"); 
        return 1; 
    } 
    if (argv[2]) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }
    if (chdir(d) != 0) { 
        perror("cd"); 
        return 1; 
    } 
    return 0; 
}

static int bi_pwd(char **argv) { 
    (void)argv; 
    char buf[4096]; 
    if (getcwd(buf, sizeof(buf))) 
        puts(buf); 
    else 
        perror("pwd"); 
    return 0; 
}

// Wait for all background processes to complete
static void wait_for_background_jobs(void) {
    printf("Waiting for background processes to complete...\n");
    fflush(stdout);
    
    // Keep reaping until no more background jobs
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) >= 0) {
        if (pid == 0) {
            // No more children ready, but some might still be running
            usleep(100000); // Wait 0.1 seconds
            continue;
        }
        // Successfully reaped a child
        jobs_mark_done(getpgid(pid));
    }
    
    // Final check - wait for any remaining processes
    while (waitpid(-1, &status, WNOHANG | WUNTRACED) > 0) {
        // Continue until no more children
    }
}

static int bi_exit(char **argv) { 
    int code = 0; 
    if (argv[1]) code = atoi(argv[1]); 
    
    // Wait for background processes
    wait_for_background_jobs();
    
    // Print command history
    print_command_history();
    
    exit(code); 
}

static int bi_export(char **argv) { 
    if (!argv[1]) { 
        fprintf(stderr, "export: NAME=VALUE required\n"); 
        return 1; 
    } 
    char *eq = strchr(argv[1], '='); 
    if (!eq) { 
        fprintf(stderr, "export: NAME=VALUE required\n"); 
        return 1; 
    } 
    *eq = 0; 
    const char *n = argv[1], *v = eq + 1; 
    if (setenv(n, v, 1) != 0) { 
        perror("export"); 
        return 1; 
    } 
    return 0; 
}

static int bi_unset(char **argv) { 
    if (!argv[1]) { 
        fprintf(stderr, "unset: NAME required\n"); 
        return 1; 
    } 
    if (unsetenv(argv[1]) != 0) { 
        perror("unset"); 
        return 1; 
    } 
    return 0; 
}

extern void jobs_list(void); 
extern int jobs_fg(int id);

static int bi_jobs(char **argv) { 
    (void)argv; 
    jobs_list(); 
    return 0; 
}

static int bi_fg(char **argv) { 
    if (!argv[1]) { 
        fprintf(stderr, "fg: job id required\n"); 
        return 1; 
    } 
    int id = atoi(argv[1]); 
    return jobs_fg(id); 
}

static int bi_help(char **argv) { 
    (void)argv; 
    puts("Built-ins: cd, pwd, exit, export NAME=VALUE, unset NAME, jobs, fg <id>, help"); 
    return 0; 
}

typedef struct { const char *name; int (*fn)(char **); } entry_t;
static entry_t table[] = { 
    {"cd", bi_cd}, {"pwd", bi_pwd}, {"exit", bi_exit}, {"export", bi_export}, 
    {"unset", bi_unset}, {"jobs", bi_jobs}, {"fg", bi_fg}, {"help", bi_help}, 
    {NULL, NULL} 
};

bool is_builtin(const char *name) { 
    for (entry_t *e = table; e->name; ++e) 
        if (strcmp(e->name, name) == 0) 
            return true; 
    return false; 
}

int run_builtin(char **argv) { 
    for (entry_t *e = table; e->name; ++e) 
        if (strcmp(e->name, argv[0]) == 0) 
            return e->fn(argv); 
    return 1; 
}