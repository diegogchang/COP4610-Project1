#ifndef MINISH_H
#define MINISH_H
#include <sys/types.h>
typedef enum { JOB_RUNNING=0, JOB_DONE=1 } job_status_t;
typedef struct job { int id; pid_t pgid; char *cmdline; job_status_t status; } job_t;
void jobs_init(void); void jobs_add(pid_t pgid, const char *cmdline);
int jobs_mark_done(pid_t pgid); void jobs_list(void); int jobs_fg(int id);
void jobs_reap_zombies(void); void jobs_free(void);
#endif
