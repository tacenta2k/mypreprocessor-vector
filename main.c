// mypreprocessor
// usage: ./mypreprocessor test.c
#include <stdio.h>
#include <string.h>
#include "comment_removal.h"
#include "file_inclusion.h"
#include "macro_handler.h"

#define SIZE 500

int main(int argc, char *argv[]) {
    char out[SIZE];
    int len;
    FILE *f;

    if (argc != 2) {
        printf("usage: %s file.c\n", argv[0]);
        return 1;
    }

    len = strlen(argv[1]);
    if (len < 3 || argv[1][len - 2] != '.' || argv[1][len - 1] != 'c') {
        printf("%s is not a .c file\n", argv[1]);
        return 1;
    }
    if (len >= SIZE) {             // too long
        printf("file name is too long\n");
        return 1;
    }

    f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("cannot open %s\n", argv[1]);
        return 1;
    }
    fclose(f);

    // test.c -> test.i
    strcpy(out, argv[1]);
    out[len - 1] = 'i';

    // step 1
    include_files(argv[1], "tmp1.txt");

    // step 2
    remove_comments("tmp1.txt", "tmp2.txt");

    // step 3
    process_macros("tmp2.txt", out);

    remove("tmp1.txt");
    remove("tmp2.txt");

    printf("created %s\n", out);
    return 0;
}
