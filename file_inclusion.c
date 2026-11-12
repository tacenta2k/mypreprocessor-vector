// pastes #include files
#include <stdio.h>
#include <string.h>
#include "file_inclusion.h"

void include_files(char in[], char out[]) {
    FILE *fp = fopen(in, "r");
    FILE *fq = fopen(out, "w");
    FILE *fh;
    char line[500];
    char path[500];
    int i, k, c;

    if (fp == NULL || fq == NULL)
        return;

    while (fgets(line, 500, fp) != NULL) {

        if (strncmp(line, "#include", 8) != 0) {
            fputs(line, fq);
            continue;
        }

        i = 8;
        while (line[i] == ' ' || line[i] == '\t')
            i++;

        // <file> from /usr/include, "file" from this folder
        if (line[i] == '<') {
            strcpy(path, "/usr/include/");
        } else if (line[i] == '"') {
            strcpy(path, "");
        } else {
            fputs(line, fq);
            continue;
        }
        i++;

        // file name goes into path
        k = strlen(path);
        while (line[i] != '>' && line[i] != '"' && line[i] != '\n' &&
               line[i] != '\0' && k < 498) {
            path[k] = line[i];
            k++;
            i++;
        }
        path[k] = '\0';

        fh = fopen(path, "r");
        if (fh == NULL) {
            printf("cannot open %s\n", path);
            fputs(line, fq);
            continue;
        }

        // paste the file
        while ((c = fgetc(fh)) != EOF)
            fputc(c, fq);
        fputc('\n', fq);

        fclose(fh);
    }

    fclose(fp);
    fclose(fq);
}
