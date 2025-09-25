mod util;
mod expand;
mod pathsearch;
mod parser;
mod exec;
mod jobs;
mod builtins;

use crate::builtins::{try_builtin, BuiltinResult, graceful_exit};
use crate::exec::execute_pipeline;
use crate::expand::expand_tokens;
use crate::jobs::{JobTable, reap_finished_background};
use crate::parser::{parse_command_line, split_whitespace_simple};
use crate::util::{read_line, prompt};

fn main() {
    let mut history: Vec<String> = Vec::new();
    let mut jobs = JobTable::new();

    loop {
        reap_finished_background(&mut jobs);
        print!("{}", prompt());
        let _ = std::io::Write::flush(&mut std::io::stdout());

        let line = match read_line() {
            Ok(s) => s.trim().to_string(),
            Err(_) => break,
        };
        if line.is_empty() { continue; }

        let tokens_raw = split_whitespace_simple(&line);
        if tokens_raw.is_empty() { continue; }

        let tokens = match expand_tokens(tokens_raw) {
            Ok(t) => t,
            Err(e) => { eprintln!("{e}"); continue; }
        };

        match try_builtin(&tokens, &mut history, &mut jobs, &line) {
            BuiltinResult::Handled => { history.push(line.clone()); continue; }
            BuiltinResult::ExitRequested => { graceful_exit(&history, &mut jobs); return; }
            BuiltinResult::ErrorPrinted => { continue; }
            BuiltinResult::NotBuiltin => {}
        }

        let cmdline = match parse_command_line(&tokens) {
            Ok(c) => c,
            Err(e) => { eprintln!("{e}"); continue; }
        };

        match execute_pipeline(&cmdline, &mut jobs, &line) {
            Ok(valid) => { if valid { history.push(line.clone()); } }
            Err(e) => eprintln!("{e}"),
        }

        reap_finished_background(&mut jobs);
    }
}
