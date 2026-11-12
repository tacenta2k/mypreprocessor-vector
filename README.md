# mypreprocessor

A small C project that shows what a C preprocessor does.  Give it a `.c`
file and it writes a `.i` file with three clean-up steps applied:

| step | what happens | example |
|------|--------------|---------|
| 1. include | a line like `#include "myheader.h"` is replaced by the contents of that file | |
| 2. comments | `// ...` and block comments are removed | |
| 3. macros | names created with `#define` are replaced by their value | `PI` becomes `3.14` |

```
test.c   --includes-->  --comments-->  --macros-->  test.i
```

## Quick start

You need a C compiler (`gcc` or `clang`) and `make`.

```bash
make                    # build the program
./mypreprocessor test.c # make test.i from the example file
cat test.i              # look at the result
```

`test.i` is a normal C file, so you can read it, or compile it with
`gcc -c test.i`.

Other useful commands:

```bash
make run    # build, then run it on test.c
make clean  # delete the program, the .o files and the .i files
sh tests/run_tests.sh   # run the test suite (nothing to install)
```

## What the program understands

```c
#include <stdio.h>          /* found in /usr/include */
#include "myheader.h"       /* found next to the .c file */

#define PI 3.14             /* simple macro */
#define ADD(a, b) ((a)+(b)) /* macro with parameters */
#define SIZE 10
#define HALF (SIZE / 2)     /* a macro used inside another macro */
```

Names inside strings are never touched, so `printf("PI = %f", PI)` keeps
the text `PI = %f` at the start and only replaces the `PI` after the comma.

Other `#` lines (`#ifdef`, `#pragma`, ...) are copied unchanged.  They
belong to the compiler's own preprocessor, not to this project.

## The files, in the order you should read them

| file | what it does |
|------|--------------|
| `main.c` | reads the command line and runs the three steps |
| `util.h` / `util.c` | tiny helpers used everywhere: safe string building, "is this a letter?", path joining, ... |
| `comment_removal.h` / `.c` | step 1: removes comments |
| `file_inclusion.h` / `.c` | step 2: pastes in `#include` files |
| `macro_handler.h` / `.c` | step 3: stores `#define`s and replaces names |
| `test.c`, `myheader.h` | a sample input file |
| `tests/run_tests.sh` | the test suite |

The three steps are chained with `tmpfile()`, so the program never leaves
temporary files behind.  The only file it writes is the `.i` file.

## How the pieces fit together

```
main.c
  |
  |-- include_files()      file_inclusion.c   -> temp file 1
  |-- remove_comments()    comment_removal.c  -> temp file 2
  |-- process_macros()     macro_handler.c    -> test.i
```

Every step reads from a `FILE *` and writes to a `FILE *`.  That is why
they can be chained so easily, and why each one can be tested on its own.

## Ideas for extending it

Good next steps if you want to keep working on this:

1. `#define` values that continue on the next line with a trailing `\`.
2. `#undef NAME` (forget a macro again).
3. A real `#if` / `#ifdef` evaluator instead of copying those lines.
4. `#` (stringify) and `##` (paste two names together) inside macros.
5. A `-o output.i` option to choose the output file name.

## Limits

This is a learning project, not a replacement for `cpp`.  In particular:

* System headers are searched in `/usr/include`, `/usr/include/x86_64-linux-gnu`,
  `/usr/local/include` and the folder `gcc` reports for itself.
* A header is only pasted in once per run, the same guard that `#ifndef`
  gives real headers.
* A `.c` file name must be at most 511 characters (a clear error is shown).
* `#include` paths must be at most 511 characters.
