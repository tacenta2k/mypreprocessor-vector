#include <stdio.h>
#include <string.h>
#include "file_inclusion.h"

void include_files(char in_name[], char out_name[]) {
    FILE *in = fopen(in_name, "r");
    FILE *out = fopen(out_name, "w");
    FILE *header;
    char line[1000], path[1024];
    int i, k, ch;

    while (fgets(line, sizeof(line), in) != NULL) {
        if (strncmp(line, "#include", 8) != 0) {
            fputs(line, out);
            continue;
        }

        i = 8;
        while (line[i] == ' ' || line[i] == '\t')
            i++;

        // <file> is taken from /usr/include, "file" from the current folder
        if (line[i] == '<') {
            strcpy(path, "/usr/include/");
        } else if (line[i] == '"') {
            strcpy(path, "");
        } else {
            fputs(line, out);
            continue;
        }
        i++;

        k = strlen(path);
        while (line[i] != '>' && line[i] != '"' && line[i] != '\n' && line[i] != '\0') {
            path[k] = line[i];
            k++;
            i++;
        }
        path[k] = '\0';

        header = fopen(path, "r");
        if (header == NULL) {
            printf("Warning: cannot open %s\n", path);
            fputs(line, out);
            continue;
        }

        while ((ch = fgetc(header)) != EOF)
            fputc(ch, out);
        fputc('\n', out);
        fclose(header);
    }

    fclose(in);
    fclose(out);
}
