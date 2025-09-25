use std::env;

pub fn expand_tokens(mut tokens: Vec<String>) -> Result<Vec<String>, String> {
    for t in tokens.iter_mut() {
        if t.starts_with('$') && t.len() > 1 {
            let key = &t[1..];
            *t = env::var(key).unwrap_or_default();
        }
        if t == "~" {
            *t = env::var("HOME").unwrap_or_default();
        } else if t.starts_with("~/") {
            let home = env::var("HOME").unwrap_or_default();
            *t = format!("{}/{}", home, &t[2..]);
        }
    }
    Ok(tokens)
}
