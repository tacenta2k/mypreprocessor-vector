// pastes #include files
#include <stdio.h>
#include <string.h>
#include "file_inclusion.h"

#define MAXFILES 20

char seen[MAXFILES][500];
int seen_count = 0;

// find #include, -1 if none
int include_start(char line[]) {
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    if (strncmp(&line[i], "#include", 8) == 0)
        return i + 8;
    return -1;
}

// already pasted?
int was_pasted(char path[]) {
    int i;
    for (i = 0; i < seen_count; i++) {
        if (strcmp(seen[i], path) == 0)
            return 1;
    }
    return 0;
}

// paste file and its includes
void paste_file(FILE *fq, char path[], char dir[]) {
    FILE *fh = fopen(path, "r");
    char line[500];
    char full[500];
    char name[500];
    char newdir[500];
    int i, k;

    if (fh == NULL)
        return;

    if (seen_count < MAXFILES)
        strcpy(seen[seen_count++], path);

    while (fgets(line, 500, fh) != NULL) {

        i = include_start(line);
        if (i == -1) {
            fputs(line, fq);
            continue;
        }
        while (line[i] == ' ' || line[i] == '\t')
            i++;

        if (line[i] == '<') {
            strcpy(full, "/usr/include/");
        } else if (line[i] == '"') {
            strcpy(full, dir);
            if (dir[0] != '\0')
                strcat(full, "/");
        } else {
            fputs(line, fq);
            continue;
        }
        i++;

        k = strlen(full);
        while (line[i] != '>' && line[i] != '"' && line[i] != '\n' &&
               line[i] != '\0' && k < 498) {
            full[k] = line[i];
            k++;
            i++;
        }
        full[k] = '\0';

        // folder for its "..." includes
        strcpy(name, full);
        for (k = strlen(name) - 1; k >= 0; k--) {
            if (name[k] == '/') {
                name[k] = '\0';
                break;
            }
        }
        strcpy(newdir, (k >= 0) ? name : "");

        if (was_pasted(full)) {
            continue;
        }
        if (fopen(full, "r") == NULL) {
            printf("skipped %s (not found)\n", full);
            fputs(line, fq);
            continue;
        }
        paste_file(fq, full, newdir);
        fputc('\n', fq);
    }

    fclose(fh);
}

void include_files(char in[], char out[]) {
    FILE *fp = fopen(in, "r");
    FILE *fq = fopen(out, "w");
    char line[500];
    char path[500];
    int i, k;

    if (fp == NULL || fq == NULL)
        return;

    seen_count = 0;

    while (fgets(line, 500, fp) != NULL) {

        i = include_start(line);
        if (i == -1) {
            fputs(line, fq);
            continue;
        }
        while (line[i] == ' ' || line[i] == '\t')
            i++;

        if (line[i] == '<') {
            strcpy(path, "/usr/include/");
        } else if (line[i] == '"') {
            strcpy(path, "");
        } else {
            fputs(line, fq);
            continue;
        }
        i++;

        k = strlen(path);
        while (line[i] != '>' && line[i] != '"' && line[i] != '\n' &&
               line[i] != '\0' && k < 498) {
            path[k] = line[i];
            k++;
            i++;
        }
        path[k] = '\0';

        if (fopen(path, "r") == NULL) {
            printf("skipped %s (not found)\n", path);
            fputs(line, fq);
            continue;
        }
        if (was_pasted(path))
            continue;

        paste_file(fq, path, "");
        fputc('\n', fq);
    }

    fclose(fp);
    fclose(fq);
}
