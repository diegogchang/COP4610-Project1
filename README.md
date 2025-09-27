# COP4610 Project 1: **minish** — A Minimal UNIX-like Shell

# Overview

**minish** is a small, portable command shell you run inside a terminal (Ubuntu, WSL, linprog). It parses commands, runs programs in `$PATH`, supports pipelines, redirection, background jobs, and several built-ins.

### Implemented Features

* **Programs** via `execvp()` with `$PATH` lookup
* **Pipelines**: `cmd1 | cmd2 | cmd3`
* **Redirection**: `cmd < in`, `cmd > out`, `cmd >> out`
* **Background jobs**: `cmd &`
* **Built-ins**: `cd`, `pwd`, `exit`, `export NAME=VALUE`, `unset NAME`, `jobs`, `fg <id>`, `help`
* **Globbing** with `glob(3)` (no extra libs needed on Linux)
* **Env expansion** inside tokens: `${VAR}`
* **Job table** with zombie reaping via `SIGCHLD`

## Repository Structure (per syllabus)

root/
├── bin/          # executable output 
├── include/      # headers (.h)
├── obj/          # object files 
├── src/          # sources (.c)
├── Makefile
└── README.md

## How to Build & Run (Ubuntu/WSL/linprog)

1. **Install toolchain** (first time):

   ```bash
   sudo apt update
   sudo apt install -y build-essential valgrind
   ```
2. **Normalize line endings** (important if edited on Windows):

   ```bash
   sed -i 's/\r$//' Makefile src/*.c include/*.h
   ```
3. **Build**:

   ```bash
   make clean
   make
   ```
4. **Run**:

   ```bash
   ./bin/minish
   ```

### Quick Demo (inside minish)

```bash
help
pwd
ls -l | grep '^d' > dirs.txt
cat < dirs.txt | wc -l
sleep 15 &     # background
jobs
fg 1
export NAME=world
echo hello ${NAME}
exit          # or Ctrl-D to quit
```

## File Listing

* `src/main.c` — entrypoint, REPL loop, signal hooks (`SIGINT` ignore, `SIGCHLD` reap)
* `src/parser.c` — tokenization, quotes, `|  <  >  >>  &`, env expansion, AST (pipeline list)
* `src/exec.c` — pipeline creation, `fork/execvp`, `dup2`, redirection, groups, foreground/background
* `src/builtins.c` — built-ins (`cd`, `pwd`, `exit`, `export`, `unset`, `jobs`, `fg`, `help`)
* `src/jobs.c` — simple job table: add/list/fg/reap/free
* `src/strutil.c` — safe allocs, `trim`, small string helpers
* `include/*.h` — public headers for the above modules
* `Makefile` — linprog-friendly, outputs to `bin/`, objects to `obj/`
* `bin/` — generated executable `minish` (not committed)
* `obj/` — generated `.o` files (not committed)

## Makefile Notes

* **C11**, `-Wall -Wextra -Wpedantic`, includes `-D_XOPEN_SOURCE=700`.
* **No extra libraries required** for `glob()` on Linux:

  ```make
  LDLIBS :=
  ```
* Common targets:

  ```bash
  make         # build bin/minish
  make run     # build + run ./bin/minish
  make clean   # remove bin/ and obj/
  make debug   # rebuild with -O0 -g3
  make memcheck  # run under valgrind (if installed)
  ```

## Known Bugs / Limitations (Documented)

* **TTY job control** is minimal: `fg` waits on the process group, but no `tcsetpgrp()` / `Ctrl-Z` job suspension management.
* **History / readline** not implemented (simple `getline` loop).
* **Command substitution** like `$(...)` and backticks not implemented.
* **Quoting** handles `'single'` and `"double"` quotes; no escapes inside quotes beyond closing quotes.

## Special Considerations for Grading

* Builds with **GCC** and standard **POSIX** APIs; no non-portable deps.
* Executable is produced in **`bin/`** and **not** committed.
* Source split is modular and passes strict warnings.
* Tested on Ubuntu/WSL and configured to run on **linprog**.

## Division of Labor 

**Part 1 Prompt** – Samuel Marcano
**Part 2 Env Vars** – Francisco De La Espriella
**Part 3 Tilde** – Diego Chang
**Part 4 $PATH** – Francisco De La Espriella
**Part 5 External Exec** – Diego Chang
**Part 6 I/O Redirection** – Samuel Marcano
**Part 7 Piping** – Francisco De La Espriella
**Part 8 Background** – Diego Chang
**Part 9 Internal Commands** – Samuel Marcano

**Extra credit**

Work divided by all 3 members

## Development Log (Each Member)

* Samuel Marcano

2025-09-14: Implemented command prompt display (Part 1)

2025-09-21: Added I/O redirection handling (<, >, >>) (Part 6)

2025-09-25: Implemented internal commands (cd, exit, pwd, export/unset) (Part 9)

* Francisco De La Espriella

2025-09-14: Implemented environment variable expansion (Part 2)

2025-09-15: Added $PATH search for executables (Part 4)

2025-09-21: Implemented pipelines (pipe, dup2, execvp) (Part 7)

* Diego Chang

2025-09-14: Implemented tilde (~) expansion (Part 3)

2025-09-16: Implemented external command execution (Part 5)

2025-09-23: Added background process handling (&, SIGCHLD reaping) (Part 8)

## Group Meetings

* 2025-09-14 (Discord, 1h): Initial integration of Parts 1–3; discussed prompt, env var, and tilde expansion.

* 2025-09-21 (Dirac, 1.5h): Reviewed I/O redirection, PATH search, and external execution; ran first integration tests.

## Troubleshooting

* **`missing separator` in Makefile**
  Convert CRLF to LF and ensure recipe lines start with **tabs**:

  ```bash
  sed -i 's/\r$//' Makefile
  ```

* **`execvp: No such file or directory` after typing `q`**
  `q` is not a program. Use `exit` or Ctrl-D 

* **Linker error for `-lglob`**
  Remove it. Linux provides `glob()` in libc; `LDLIBS` should be empty.


## Extra Credit (Language Diversification)

Extra credit: Unix-like minish Shell recreated in RUST with same functionalities as C minish shell

Rust Shell inside rust_project folder
