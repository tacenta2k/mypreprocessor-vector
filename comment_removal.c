// strips comments, one char at a time
#include <stdio.h>
#include "comment_removal.h"

// where we are while reading
enum {
    CODE,      // normal code
    IN_STRING, // inside "..."
    IN_CHAR,   // inside '...'
    LINE_COMM, // after //
    BLOCK_COMM // block comment
};

void remove_comments(FILE *in, FILE *out) {
    int state = CODE;
    int prev = 0; // last char, needed to spot the closing */
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
                int next = fgetc(in);
                if (next == '/') {
                    state = LINE_COMM;
                } else if (next == '*') {
                    fputc(' ', out); // space so two words do not merge
                    state = BLOCK_COMM;
                    prev = 0;
                } else {
                    fputc('/', out);
                    if (next != EOF)
                        ungetc(next, in); // not a comment
                }
            } else {
                fputc(ch, out);
            }

        } else if (state == IN_STRING || state == IN_CHAR) {
            fputc(ch, out);
            if (ch == '\\') {
                int next = fgetc(in); // escaped char, keep it
                if (next != EOF)
                    fputc(next, out);
            } else if ((state == IN_STRING && ch == '"') ||
                       (state == IN_CHAR && ch == '\'')) {
                state = CODE;
            } else if (ch == '\n') {
                state = CODE; // broken string, start over
            }

        } else if (state == LINE_COMM) {
            if (ch == '\\') {
                int next = fgetc(in);
                if (next == '\n') {
                    fputc('\n', out); // continuation, keep the line break
                } else if (next != EOF) {
                    ungetc(next, in);
                }
            } else if (ch == '\n') {
                fputc('\n', out);
                state = CODE;
            }

        } else { // BLOCK_COMM
            if (ch == '/' && prev == '*') {
                state = CODE;
            } else if (ch == '\n') {
                fputc('\n', out); // keep line numbers
            }
            prev = ch;
        }
    }
}
