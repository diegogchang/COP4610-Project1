use std::env;
use std::path::{Path, PathBuf};

fn is_executable(p: &Path) -> bool {
    if let Ok(meta) = std::fs::metadata(p) {
        meta.is_file() && nix::unistd::access(p, nix::unistd::AccessFlags::X_OK).is_ok()
    } else { false }
}

pub fn resolve_executable(cmd: &str) -> Option<PathBuf> {
    if cmd.contains('/') {
        let p = PathBuf::from(cmd);
        return if is_executable(&p) { Some(p) } else { None };
    }
    let path = env::var("PATH").unwrap_or_else(|_| "/bin:/usr/bin".into());
    for dir in path.split(':') {
        let candidate = Path::new(dir).join(cmd);
        if is_executable(&candidate) { return Some(candidate); }
    }
    None
}
