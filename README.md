# mypreprocessor

A C preprocessor mini-project. It reads a `.c` file and creates a `.i` file after including header files, removing comments and replacing macros.

## Files

- `main.c` - takes the input file name and runs the 3 steps
- `file_inclusion.c` / `file_inclusion.h` - replaces `#include <file>` and `#include "file"` with the file contents
- `comment_removal.c` / `comment_removal.h` - removes `//` and `/* ... */` comments
- `macro_handler.c` / `macro_handler.h` - stores `#define` macros (simple macros and macros with arguments) and replaces them in the code
- `Makefile` - builds the `mypreprocessor` executable
- `test.c`, `myheader.h` - sample input for testing

## Build and Run

```bash
make
./mypreprocessor test.c
```

This creates `test.i`.

To remove compiled files:

```bash
make clean
```
