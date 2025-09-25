#[derive(Debug, Clone)]
pub struct Stage {
    pub argv: Vec<String>,
    pub infile: Option<String>,
    pub outfile: Option<String>,
}

#[derive(Debug, Clone)]
pub struct CommandLine {
    pub stages: Vec<Stage>,
    pub background: bool,
}

pub fn split_whitespace_simple(line: &str) -> Vec<String> {
    line.split_whitespace().map(|s| s.to_string()).collect()
}

pub fn parse_command_line(tokens: &[String]) -> Result<CommandLine, String> {
    let mut toks = tokens.to_vec();
    let mut background = false;
    if toks.last().map(|s| s.as_str()) == Some("&") {
        background = true;
        toks.pop();
        if toks.is_empty() { return Err("syntax: '&' with no command".into()); }
    }

    let mut stages = Vec::new();
    let mut current: Vec<String> = Vec::new();
    for t in toks {
        if t == "|" {
            if current.is_empty() { return Err("syntax: empty stage before '|'".into()); }
            stages.push(parse_stage(&current)?);
            current = Vec::new();
        } else {
            current.push(t);
        }
    }
    if current.is_empty() { return Err("syntax: empty final stage".into()); }
    stages.push(parse_stage(&current)?);

    if stages.len() > 3 { return Err("too many pipes (max 2)".into()); }
    Ok(CommandLine { stages, background })
}

fn parse_stage(stage_toks: &[String]) -> Result<Stage, String> {
    let mut argv = Vec::new();
    let mut infile: Option<String> = None;
    let mut outfile: Option<String> = None;

    let mut i = 0;
    while i < stage_toks.len() {
        match stage_toks[i].as_str() {
            "<" => {
                if i + 1 >= stage_toks.len() { return Err("syntax: missing file after '<'".into()); }
                infile = Some(stage_toks[i + 1].clone());
                i += 2;
            }
            ">" => {
                if i + 1 >= stage_toks.len() { return Err("syntax: missing file after '>'".into()); }
                outfile = Some(stage_toks[i + 1].clone());
                i += 2;
            }
            tok => { argv.push(tok.to_string()); i += 1; }
        }
    }
    if argv.is_empty() { return Err("syntax: empty stage".into()); }
    Ok(Stage { argv, infile, outfile })
}
