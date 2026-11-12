/* comment_removal.c -- remove one-line and block comments from C code.
 *
 * The whole file is read one character at a time.  A small "state" variable
 * remembers where we are, which keeps the code short and easy to follow.
 */
#include <stdio.h>
#include "comment_removal.h"

/* The five places the reader can be in. */
enum {
    CODE,       /* ordinary C code                                  */
    IN_STRING,  /* inside a "..." text                               */
    IN_CHAR,    /* inside a '...' character                          */
    LINE_COMM,  /* after //, up to the end of the line               */
    BLOCK_COMM  /* inside a block comment                            */
};

void remove_comments(FILE *in, FILE *out) {
    int state = CODE;
    int prev = 0; /* the character before the current one, for BLOCK_COMM */
    int ch;

    while ((ch = fgetc(in)) != EOF) {
        if (state == CODE) {
            if (ch == '"') {
                state = IN_STRING;
                fputc(ch, out);
            } else if (ch == '\'') {
                state = IN_CHAR;
                fputc(ch, out);
            } else if (ch == '/') {
                int next = fgetc(in); /* look at the character after '/' */
                if (next == '/') {
                    state = LINE_COMM;
                } else if (next == '*') {
                    /* Put a space where the comment was, so that two
                     * names do not run together into one. */
                    fputc(' ', out);
                    state = BLOCK_COMM;
                    prev = 0;
                } else {
                    fputc('/', out);
                    if (next != EOF)
                        ungetc(next, in); /* it was not a comment after all */
                }
            } else {
                fputc(ch, out);
            }

        } else if (state == IN_STRING || state == IN_CHAR) {
            fputc(ch, out);
            if (ch == '\\') {
                /* keep the next character exactly, e.g. \" does not end the
                 * string */
                int next = fgetc(in);
                if (next != EOF)
                    fputc(next, out);
            } else if ((state == IN_STRING && ch == '"') ||
                       (state == IN_CHAR && ch == '\'')) {
                state = CODE; /* closing quote */
            } else if (ch == '\n') {
                state = CODE; /* broken string, recover on the next line */
            }

        } else if (state == LINE_COMM) {
            if (ch == '\\') {
                int next = fgetc(in);
                if (next == '\n') {
                    /* a backslash at the end of the line means the comment
                     * goes on.  Keep the newline so line numbers survive. */
                    fputc('\n', out);
                } else if (next != EOF) {
                    ungetc(next, in);
                }
            } else if (ch == '\n') {
                fputc('\n', out);
                state = CODE;
            }

        } else { /* BLOCK_COMM */
            if (ch == '/' && prev == '*') {
                state = CODE; /* found the closing marker */
            } else if (ch == '\n') {
                fputc('\n', out); /* keep line numbers correct */
            }
            prev = ch;
        }
    }
}
