#include <stdio.h>
#include "comment_removal.h"

void remove_comments(char in_name[], char out_name[]) {
    FILE *in = fopen(in_name, "r");
    FILE *out = fopen(out_name, "w");
    int ch, next, prev, quote;

    while ((ch = fgetc(in)) != EOF) {
        if (ch == '"' || ch == '\'') {
            // string or character, copy it as it is
            quote = ch;
            fputc(ch, out);
            while ((ch = fgetc(in)) != EOF) {
                fputc(ch, out);
                if (ch == '\\')
                    fputc(fgetc(in), out);
                else if (ch == quote)
                    break;
            }
        } else if (ch == '/') {
            next = fgetc(in);
            if (next == '/') {
                // single line comment
                ch = fgetc(in);
                while (ch != '\n' && ch != EOF)
                    ch = fgetc(in);
                fputc('\n', out);
            } else if (next == '*') {
                // multi line comment
                prev = 0;
                ch = fgetc(in);
                while (ch != EOF && !(prev == '*' && ch == '/')) {
                    if (ch == '\n')
                        fputc('\n', out);
                    prev = ch;
                    ch = fgetc(in);
                }
                fputc(' ', out);
            } else {
                fputc(ch, out);
                ungetc(next, in);
            }
        } else {
            fputc(ch, out);
        }
    }

    fclose(in);
    fclose(out);
}
