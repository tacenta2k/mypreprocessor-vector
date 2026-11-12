#include <stdio.h>
#include <string.h>
#include "comment_removal.h"
#include "file_inclusion.h"
#include "macro_handler.h"

int main(int argc, char *argv[]) {
    char out_name[500];
    int len;
    FILE *fp;

    if (argc != 2) {
        printf("Usage: %s inputfile.c\n", argv[0]);
        return 1;
    }

    len = strlen(argv[1]);
    if (len < 3 || argv[1][len - 2] != '.' || argv[1][len - 1] != 'c') {
        printf("Error: %s is not a .c file\n", argv[1]);
        return 1;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error: cannot open %s\n", argv[1]);
        return 1;
    }
    fclose(fp);

    // test.c -> test.i
    strcpy(out_name, argv[1]);
    out_name[len - 1] = 'i';

    include_files(argv[1], "temp1.tmp");
    remove_comments("temp1.tmp", "temp2.tmp");
    process_macros("temp2.tmp", out_name);

    remove("temp1.tmp");
    remove("temp2.tmp");

    printf("Created %s\n", out_name);
    return 0;
}
