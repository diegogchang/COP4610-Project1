use crate::jobs::{JobTable, wait_all_background, print_jobs};
use nix::unistd::chdir;
use std::env;
use std::path::Path;

pub enum BuiltinResult {
    Handled,
    NotBuiltin,
    ExitRequested,
    ErrorPrinted,
}

pub fn try_builtin(tokens: &[String], _history: &mut Vec<String>, jobs: &mut JobTable, _line: &str) -> BuiltinResult {
    if tokens.is_empty() { return BuiltinResult::NotBuiltin; }
    match tokens[0].as_str() {
        "exit" => BuiltinResult::ExitRequested,
        "cd" => {
            if tokens.len() > 2 {
                eprintln!("cd: too many arguments");
                return BuiltinResult::ErrorPrinted;
            }
            let target = if tokens.len() == 1 {
                match env::var("HOME") {
                    Ok(h) => h,
                    Err(_) => { eprintln!("cd: $HOME not set"); return BuiltinResult::ErrorPrinted; }
                }
            } else { tokens[1].clone() };
            if let Err(e) = chdir(Path::new(&target)) {
                eprintln!("cd: {e}");
                return BuiltinResult::ErrorPrinted;
            }
            BuiltinResult::Handled
        }
        "jobs" => { print_jobs(jobs); BuiltinResult::Handled }
        _ => BuiltinResult::NotBuiltin,
    }
}

pub fn graceful_exit(history: &Vec<String>, jobs: &mut JobTable) {
    wait_all_background(jobs);
    if history.is_empty() {
        println!("No valid commands executed.");
        return;
    }
    if history.len() < 3 {
        println!("{}", history.last().unwrap());
        return;
    }
    for cmd in &history[history.len()-3..] { println!("{cmd}"); }
}
