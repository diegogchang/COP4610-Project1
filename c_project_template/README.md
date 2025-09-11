# COP4610 Project <N>: <Short Title>

## Team & Division of Labor (Before)
- **Member 1 (FSUID / email)** — planned: design of X, module A
- **Member 2 (FSUID / email)** — planned: implementation of Y, module B
- **Member 3 (FSUID / email)** — planned: testing/CI, integration, documentation

## File Listing
- `src/main.c` — program entry; parses args, calls core logic
- `src/app.c` — core functionality (example: array ops)
- `include/app.h` — exports symbols for `app.c`
- `Makefile` — builds to `bin/`, objects to `obj/`, headers in `include/`
- `.gitignore` — excludes binaries/objects/IDE files
- `README.md` — this file

## How to Compile & Run (linprog verified)
```bash
# on linprog:
make            # builds bin/proj1
./bin/proj1     # run

# optional:
make clean
make debug
make memcheck   # requires valgrind
```

## Known Bugs & Unfinished Portions
- **Example**: If input file is empty, program returns 0 without message.
  - *Stage:* runtime
  - *First observed:* 2025-09-10
  - *Symptoms:* Silent exit
  - *Attempted fixes:* Add input validation; TODO add error code.

(Add all real issues you find. Undocumented bugs are penalized more.)

## Special Considerations for Grading
- Built and tested on **linprog** with GCC 11+.
- Executable is produced in `bin/` and never committed.

## Extra Credit (Language Diversification)
- (If applicable for *you personally* across the three projects)  
  I will implement Project X in **C** and Project Y in **Rust** to qualify for the ~1–2% final-grade bonus.

## Development Log (Each Member)
- **Member 1**  
  - 2025-09-10: Designed header layout; wrote `app_sum`
  - 2025-09-11: Fixed off-by-one in loop; added tests

- **Member 2**  
  - 2025-09-10: Set up Makefile; verified on linprog

- **Member 3**  
  - 2025-09-11: Wrote README; ran `valgrind` and recorded results

(Keep brief, factual entries with dates and what changed.)

## Group Meetings
- 2025-09-10 (30 min, Discord): finalized API for `app_run`
- 2025-09-12 (45 min, in person MCH 201): integration test + README pass

## Division of Labor (After)
- **Member 1** — implemented `app.c`, unit tests (45%)
- **Member 2** — Makefile, CI, integration (30%)
- **Member 3** — README, testing, bug triage (25%)
