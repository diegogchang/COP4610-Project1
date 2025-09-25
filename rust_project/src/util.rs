use std::env;
use std::io;

pub fn prompt() -> String {
    let user = env::var("USER").unwrap_or_else(|_| "user".into());
    let machine = env::var("MACHINE")
        .or_else(|_| env::var("HOSTNAME"))
        .unwrap_or_else(|_| "machine".into());
    let pwd = std::env::current_dir()
        .map(|p| p.display().to_string())
        .unwrap_or_else(|_| "?".into());
    format!("{user}@{machine}:{pwd}> ")
}

pub fn read_line() -> io::Result<String> {
    let mut s = String::new();
    io::stdin().read_line(&mut s)?;
    Ok(s)
}
