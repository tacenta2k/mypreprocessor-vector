# mypreprocessor

C preprocessor project. Takes a `.c` file, makes a `.i` file.

```bash
make
./mypreprocessor test.c
```

Makes `test.i`.

## Files

| file | does |
|------|------|
| main.c | runs the 3 steps |
| comment_removal.c | removes `//` and `/* */` |
| file_inclusion.c | pastes `#include` files |
| macro_handler.c | stores `#define`, replaces names |
| test.c, myheader.h | sample input |

## How it works

main.c makes two temp files:

```
test.c  ->  tmp1.txt  ->  tmp2.txt  ->  test.i
            step 1       step 2       step 3
```

Step 1 `include_files` reads test.c line by line. If a line starts with
`#include`, it opens that file and copies it into tmp1.txt. Else it just
copies the line.

Step 2 `remove_comments` reads tmp1.txt char by char. It keeps a variable
`state` to know where it is: normal code, inside a string, inside `//`, or
inside `/* */`. Only normal code is written out.

Step 3 `process_macros` reads tmp2.txt line by line. A `#define` line is
saved in an array and not written. Any other line is scanned for macro
names and they are replaced.

## Viva questions

**Why 3 files?** One job per file, easy to test and explain.

**Why temp files?** Each step reads one file and writes another, so they
stay separate. main.c deletes them at the end.

**How does it know what a name is?** `is_name_char` checks a-z, A-Z, 0-9
and `_`. Anything else ends the name.

**How are comments in strings handled?** `printf("http://x")` - the `"`
starts a string, and everything up to the closing `"` is copied without
looking for comments.

**Why is there a loop of 10 in process_macros?** A macro value can have
another macro: `#define HALF (SIZE/2)`. One pass turns HALF into
(SIZE/2), the second pass turns SIZE into 10. It stops when the line stops
changing.

**How do macros with arguments work?** `#define ADD(a,b) ((a)+(b))`
stores the name ADD, the params a and b, and the value `((a)+(b))`.
When ADD(1,2) is found, `read_args` splits `1` and `2`, and `fill_value`
copies the value but writes `1` where it sees `a` and `2` where it sees
`b`.

**Why check the argument count?** If `ADD(1)` is called with 2 params, the
code prints `ADD(1)` unchanged instead of making broken code.

## Limits

- only the 3 features in the spec: comments, includes, macros
- `#include` goes one level deep, as the spec says. A header that includes
  another header is pasted as text, not followed.
- `#if` / `#ifdef` are copied, not evaluated
- file names and include paths over 499 characters are refused
- `test.i` still has `#ifdef` lines, so gcc cannot compile it. That is
  normal for a partial preprocessor.
