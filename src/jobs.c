#include "minish.h"
#include "strutil.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_JOBS 128

static job_t *table[MAX_JOBS];
static int next_id = 1;

void jobs_init(void) { 
    memset(table, 0, sizeof(table)); 
    next_id = 1; 
}

static int find_slot(void) {
    for (int i = 0; i < MAX_JOBS; ++i) 
        if (!table[i]) 
            return i;
    return -1;
}

void jobs_add(pid_t pgid, const char *cmdline) {
    int slot = find_slot();
    if (slot < 0) return;
    
    job_t *j = xcalloc(1, sizeof(job_t));
    j->id = next_id++;
    j->pgid = pgid;
    j->cmdline = xstrdup(cmdline);
    j->status = JOB_RUNNING;
    table[slot] = j;
    
    // Print job start message in correct format: [Job number] [PID]
    printf("[%d] %d\n", j->id, (int)pgid);
    fflush(stdout);
}

int jobs_mark_done(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        job_t *j = table[i];
        if (j && j->pgid == pgid && j->status == JOB_RUNNING) { 
            j->status = JOB_DONE; 
            // Print completion message: [Job number]+ done [command line]
            printf("[%d]+ done %s\n", j->id, j->cmdline);
            fflush(stdout);
            return j->id; 
        }
    }
    return -1;
}

void jobs_list(void) {
    int found = 0;
    for (int i = 0; i < MAX_JOBS; ++i) {
        job_t *j = table[i];
        if (!j) continue;
        
        found = 1;
        // Format: [Job number]+ [PID] [command line]
        printf("[%d]+ %d %s\n",
               j->id,
               (int)j->pgid,
               j->cmdline);
    }
    
    if (!found) {
        printf("No active background processes.\n");
    }
}

static job_t *jobs_get_by_id(int id) {
    for (int i = 0; i < MAX_JOBS; ++i)
        if (table[i] && table[i]->id == id) 
            return table[i];
    return NULL;
}

int jobs_fg(int id) {
    job_t *j = jobs_get_by_id(id);
    if (!j) { 
        fprintf(stderr, "fg: no such job %d\n", id); 
        return 1; 
    }
    if (j->status == JOB_DONE) {
        fprintf(stderr, "fg: job %d already completed\n", id);
        return 1;
    }
    
    printf("%s\n", j->cmdline); // Show what command is being brought to foreground
    fflush(stdout);
    
    int status = 0;
    pid_t w;
    do { 
        w = waitpid(-j->pgid, &status, 0); 
    } while (w > 0);
    
    j->status = JOB_DONE;
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

void jobs_reap_zombies(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        pid_t pg = getpgid(pid);
        if (pg > 0) {
            jobs_mark_done(pg);
        }
    }
}

void jobs_free(void) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (table[i]) { 
            free(table[i]->cmdline); 
            free(table[i]); 
            table[i] = NULL; 
        }
    }
}