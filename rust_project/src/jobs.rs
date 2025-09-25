use nix::unistd::Pid;
use nix::sys::wait::{waitpid, WaitPidFlag, WaitStatus};

#[derive(Clone, Debug)]
pub struct Job {
    pub job_no: usize,
    pub pid: Pid,
    pub cmdline: String,
    pub active: bool,
}

#[derive(Clone, Debug)]
pub struct JobTable {
    pub jobs: Vec<Job>,
    pub total_created: usize,
}

impl JobTable {
    pub fn new() -> Self { Self { jobs: Vec::new(), total_created: 0 } }
    pub fn add_job(&mut self, pid: Pid, cmdline: &str) {
        self.total_created += 1;
        let job_no = self.total_created;
        println!("[{}] {}", job_no, pid.as_raw());
        self.jobs.push(Job { job_no, pid, cmdline: cmdline.to_string(), active: true });
    }
}

pub fn reap_finished_background(jobs: &mut JobTable) {
    loop {
        match waitpid(Pid::from_raw(-1), Some(WaitPidFlag::WNOHANG)) {
            Ok(WaitStatus::Exited(pid, _)) |
            Ok(WaitStatus::Signaled(pid, _, _)) => {
                if pid.as_raw() <= 0 { break; }
                if let Some(j) = jobs.jobs.iter_mut().find(|j| j.pid == pid && j.active) {
                    j.active = false;
                    println!("[{}] + done {}", j.job_no, j.cmdline);
                }
            }
            Ok(WaitStatus::StillAlive) => break,
            Ok(_) => break,
            Err(_) => break,
        }
    }
}

pub fn wait_all_background(jobs: &mut JobTable) {
    loop {
        if !jobs.jobs.iter().any(|j| j.active) { break; }
        reap_finished_background(jobs);
        std::thread::sleep(std::time::Duration::from_millis(30));
    }
}

pub fn print_jobs(jobs: &JobTable) {
    let mut any = false;
    for j in jobs.jobs.iter().filter(|j| j.active) {
        any = true;
        println!("[{}]+ {} {}", j.job_no, j.pid.as_raw(), j.cmdline);
    }
    if !any { println!("No active background processes."); }
}
