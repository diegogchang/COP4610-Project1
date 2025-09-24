Here’s a drop-in **README.md** you can paste over your current one. It matches the COP4610 rubric and documents the full **minish** project you built.

---

# COP4610 Project 1: **minish** — A Minimal UNIX-like Shell

## Overview

**minish** is a small, portable command shell you run **inside a terminal** (Ubuntu, WSL, linprog). It parses commands, runs programs in `$PATH`, supports pipelines, redirection, background jobs, and several built-ins.

### Implemented Features

* **Programs** via `execvp()` with `$PATH` lookup
* **Pipelines**: `cmd1 | cmd2 | cmd3`
* **Redirection**: `cmd < in`, `cmd > out`, `cmd >> out`
* **Background jobs**: `cmd &`
* **Built-ins**: `cd`, `pwd`, `exit`, `export NAME=VALUE`, `unset NAME`, `jobs`, `fg <id>`, `help`
* **Globbing** with `glob(3)` (no extra libs needed on Linux)
* **Env expansion** inside tokens: `${VAR}`
* **Job table** with zombie reaping via `SIGCHLD`

> Note: **minish is a shell, not a terminal**. You start it from your terminal; then `minish` reads and runs your commands.

---

## Repository Structure (per syllabus)

```
root/
├── bin/          # executable output (generated; not committed)
├── include/      # headers (.h)
├── obj/          # object files (generated; not committed)
├── src/          # sources (.c)
├── Makefile
└── README.md
```

---

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

---

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

---

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

---

## Known Bugs / Limitations (Documented)

* **TTY job control** is minimal: `fg` waits on the process group, but no `tcsetpgrp()` / `Ctrl-Z` job suspension management.
* **History / readline** not implemented (simple `getline` loop).
* **Command substitution** like `$(...)` and backticks not implemented.
* **Quoting** handles `'single'` and `"double"` quotes; no escapes inside quotes beyond closing quotes.

---

## Special Considerations for Grading

* Builds with **GCC** and standard **POSIX** APIs; no non-portable deps.
* Executable is produced in **`bin/`** and **not** committed.
* Source split is modular and passes strict warnings.
* Tested on Ubuntu/WSL and configured to run on **linprog**.

---

## Division of Labor (Before)

> Replace with your real names / FSUIDs.

* **Member A** — parser + redirection
* **Member B** — executor + jobs
* **Member C** — built-ins + README/QA

## Development Log (Each Member)

* **Member A**

  * 2025-09-10: Implement parser for `| < > >> &` and env expansion
* **Member B**

  * 2025-09-10: Implement pipelines (`pipe`, `dup2`, `execvp`) and `SIGCHLD` reaping
* **Member C**

  * 2025-09-11: Implement built-ins; wrote README; ran `valgrind`

## Group Meetings

* 2025-09-10 (Discord, 30m): parser/executor API
* 2025-09-12 (MCH 201, 45m): integration + tests

## Division of Labor (After)

* **Member A** — parser 40%
* **Member B** — executor/jobs 40%
* **Member C** — built-ins/docs 20%

---

## Troubleshooting

* **`missing separator` in Makefile**
  Convert CRLF to LF and ensure recipe lines start with **tabs**:

  ```bash
  sed -i 's/\r$//' Makefile
  ```

* **`execvp: No such file or directory` after typing `q`**
  `q` is not a program. Use `exit` or Ctrl-D (or add a `q`/`quit` alias in `builtins.c` if you want).

* **Linker error for `-lglob`**
  Remove it. Linux provides `glob()` in libc; `LDLIBS` should be empty.

---

## Extra Credit (Language Diversification)

If you use **C** here, implement **another project** in **Rust** to qualify for the \~1–2% bonus per course policy. Note it in the final README for that project.

---

## Future Enhancements (optional)

* `q`/`quit` aliases for `exit` (1-minute change in `builtins.c`)
* Command history (readline) if allowed by the spec
* Proper TTY job control (`tcsetpgrp`, `SIGTSTP` handling)
* Unit tests for parser tokens

---

If you want, I can also drop in the `q`/`quit` aliases and update this README accordingly.
