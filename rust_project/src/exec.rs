use crate::parser::CommandLine;
use crate::pathsearch::resolve_executable;
use crate::jobs::JobTable;

use nix::unistd::{fork, ForkResult, execv, pipe, dup2, Pid};
use nix::sys::wait::waitpid;
use nix::fcntl::{open, OFlag};
use nix::sys::stat::Mode;
use std::ffi::CString;
use std::os::unix::ffi::OsStrExt;   // for as_bytes()
use std::os::unix::io::AsRawFd;     // for OwnedFd::as_raw_fd()
use std::path::Path;
use std::fs;

fn is_regular_file(path: &str) -> bool {
    fs::metadata(path).map(|m| m.is_file()).unwrap_or(false)
}

fn open_in_read(path: &str) -> Result<i32, String> {
    open(Path::new(path), OFlag::O_RDONLY, Mode::empty())
        .map_err(|e| format!("input open error: {e}"))
}

fn open_out_write(path: &str) -> Result<i32, String> {
    open(
        Path::new(path),
        OFlag::O_CREAT | OFlag::O_WRONLY | OFlag::O_TRUNC,
        Mode::S_IRUSR | Mode::S_IWUSR,
    ).map_err(|e| format!("output open error: {e}"))
}

pub fn execute_pipeline(cmd: &CommandLine, jobs: &mut JobTable, cmdline_text: &str) -> Result<bool, String> {
    let n = cmd.stages.len();
    if n == 0 { return Err("empty pipeline".into()); }
    if n > 3 { return Err("too many pipes (max 2)".into()); }

    if let Some(ref f) = cmd.stages[0].infile {
        if !is_regular_file(f) { return Err("input: not a regular file or does not exist".into()); }
    }

    // Resolve exec paths + argv
    let mut exec_paths: Vec<CString> = Vec::with_capacity(n);
    let mut argv_list: Vec<Vec<CString>> = Vec::with_capacity(n);
    for st in &cmd.stages {
        if st.argv.is_empty() { return Err("empty stage".into()); }
        let prog = &st.argv[0];
        let path = resolve_executable(prog).ok_or_else(|| format!("{prog}: command not found"))?;
        let c_prog = CString::new(path.as_os_str().as_bytes()).unwrap();
        let c_argv: Vec<CString> = st.argv.iter().map(|s| CString::new(s.as_str()).unwrap()).collect();
        exec_paths.push(c_prog);
        argv_list.push(c_argv);
    }

    // Create pipes (OwnedFd in nix 0.28)
    let mut pipes = Vec::new(); // Vec<(OwnedFd, OwnedFd)>
    for _ in 0..(n.saturating_sub(1)) {
        pipes.push(pipe().map_err(|e| format!("pipe error: {e}"))?);
    }

    // Spawn children
    let mut pids: Vec<Pid> = Vec::new();
    for i in 0..n {
        match unsafe { fork() } {
            Ok(ForkResult::Child) => {
                // stdin: infile for first stage, else read end of previous pipe
                if i == 0 {
                    if let Some(ref infile) = cmd.stages[i].infile {
                        let fd = open_in_read(infile)?;
                        dup2(fd, 0).map_err(|e| format!("dup2 stdin: {e}"))?;
                    }
                } else {
                    let (ref r, _) = pipes[i-1];
                    dup2(r.as_raw_fd(), 0).map_err(|e| format!("dup2 stdin pipe: {e}"))?;
                }

                // stdout: outfile for last stage, else write end of this pipe
                if i == n-1 {
                    if let Some(ref outfile) = cmd.stages[i].outfile {
                        let fd = open_out_write(outfile)?;
                        dup2(fd, 1).map_err(|e| format!("dup2 stdout: {e}"))?;
                    }
                } else {
                    let (_, ref w) = pipes[i];
                    dup2(w.as_raw_fd(), 1).map_err(|e| format!("dup2 stdout pipe: {e}"))?;
                }

                // close all pipe fds in child
                for (r, w) in pipes.into_iter() { drop(r); drop(w); }

                // exec
                if let Err(e) = execv(&exec_paths[i], &argv_list[i]) {
                    eprintln!("exec error: {e}");
                    unsafe { libc::_exit(127); }
                }
                unreachable!();
            }
            Ok(ForkResult::Parent { child }) => { pids.push(child); }
            Err(e) => { return Err(format!("fork error: {e}")); }
        }
    }

    // parent closes (drops) its pipe ends
    drop(pipes);

    if cmd.background {
        if let Some(&last) = pids.last() { jobs.add_job(last, cmdline_text); }
        return Ok(true);
    }

    for pid in pids { let _ = waitpid(pid, None); }
    Ok(true)
}
