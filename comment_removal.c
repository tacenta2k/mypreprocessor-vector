// removes comments
#include <stdio.h>
#include "comment_removal.h"

void remove_comments(char in[], char out[]) {
    FILE *fp = fopen(in, "r");
    FILE *fq = fopen(out, "w");
    int c, d;

    if (fp == NULL || fq == NULL)
        return;

    while ((c = fgetc(fp)) != EOF) {

        // string or char, copy
        if (c == '"' || c == '\'') {
            int q = c;
            fputc(c, fq);
            while ((c = fgetc(fp)) != EOF) {
                fputc(c, fq);
                if (c == '\\') {
                    c = fgetc(fp);
                    if (c != EOF)
                        fputc(c, fq);
                } else if (c == q) {
                    break;
                }
            }
        }

        // maybe comment
        else if (c == '/') {
            d = fgetc(fp);

            // line comment
            if (d == '/') {
                while ((c = fgetc(fp)) != EOF && c != '\n')
                    ;
                fputc('\n', fq);
            }

            // block comment
            else if (d == '*') {
                int prev = 0;
                while ((c = fgetc(fp)) != EOF) {
                    if (c == '\n')
                        fputc('\n', fq); // keep lines
                    if (prev == '*' && c == '/')
                        break;
                    prev = c;
                }
            }

            // plain slash
            else {
                fputc(c, fq);
                if (d != EOF)
                    ungetc(d, fp);
            }
        }

        else {
            fputc(c, fq);
        }
    }

    fclose(fp);
    fclose(fq);
}
