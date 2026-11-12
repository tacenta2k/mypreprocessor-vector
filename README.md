# mypreprocessor

A small C preprocessor. It takes a `.c` file and writes a `.i` file with
three steps done: includes pasted in, comments removed, macros replaced.

```
test.c  ->  includes  ->  comments  ->  macros  ->  test.i
```

## Build and run

```bash
make
./mypreprocessor test.c
cat test.i
```

Other commands:

```bash
make run                # build and run on test.c
make clean              # remove .o, the program and .i files
sh tests/run_tests.sh   # run the tests
```

## Files

| file | what it does |
|------|--------------|
| `main.c` | reads the command line, runs the three steps |
| `util.c` `util.h` | small shared helpers (string building, paths, quotes) |
| `file_inclusion.c` | step 1, pastes in `#include` files |
| `comment_removal.c` | step 2, removes `//` and `/* */` comments |
| `macro_handler.c` | step 3, stores `#define` and replaces names |
| `test.c` `myheader.h` | example input |
| `tests/run_tests.sh` | test suite |

Read them in that order.

## What it handles

```c
#include <stdio.h>          // from /usr/include
#include "myheader.h"       // from the folder of the .c file

#define PI 3.14
#define ADD(a, b) ((a) + (b))
#define SIZE 10
#define HALF (SIZE / 2)     // macros inside macros work
```

Text inside strings is not touched. Other `#` lines (`#ifdef`, `#pragma`)
are copied as they are, since the real compiler handles those.

## Known limits

- `.c` file names and include paths up to 511 characters
- a header is pasted in only once per run
- `#define` values ending in `\` are not supported
- `#if` / `#ifdef` are copied, not evaluated
- `test.i` will not compile with gcc: it still has `#ifdef` lines, which
  need a real preprocessor. That is expected.

## Ideas to extend

1. `#undef NAME`
2. `#if` / `#ifdef` evaluation
3. `#` stringify and `##` paste inside macros
4. a `-o` option for the output name
