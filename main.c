// runs include, comments and macros on one .c file
// usage: ./mypreprocessor test.c  ->  test.i
#include <stdio.h>
#include <string.h>
#include "comment_removal.h"
#include "file_inclusion.h"
#include "macro_handler.h"
#include "util.h"

#define EXIT_USAGE 1
#define EXIT_INPUT 2
#define EXIT_IO 3

static void print_usage(const char *program) {
    printf("Usage: %s file.c\n", program);
    printf("\n");
    printf("Steps: include files, remove comments, expand macros\n");
    printf("Writes file.i next to the input file.\n");
    printf("Example: %s test.c   makes test.i\n", program);
}

static int has_c_extension(const char *name) {
    int len = (int)strlen(name);
    return len >= 2 && name[len - 2] == '.' && name[len - 1] == 'c';
}

int main(int argc, char *argv[]) {
    char out_name[MAX_PATH];
    FILE *input;
    FILE *step1;
    FILE *step2;
    FILE *output;
    int len;

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_USAGE;
    }
    if (argc > 2) {
        printf("Error: give only one file name\n\n");
        print_usage(argv[0]);
        return EXIT_USAGE;
    }
    if (!has_c_extension(argv[1])) {
        printf("Error: %s is not a .c file\n", argv[1]);
        return EXIT_USAGE;
    }
    if (strlen(argv[1]) >= MAX_PATH) {
        printf("Error: file name too long (max %d)\n", MAX_PATH - 1);
        return EXIT_USAGE;
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Error: cannot open %s\n", argv[1]);
        return EXIT_INPUT;
    }

    // test.c -> test.i, keep the folder
    copy_text(out_name, sizeof(out_name), argv[1]);
    len = (int)strlen(out_name);
    out_name[len - 1] = 'i';

    // temp files, deleted when closed
    step1 = tmpfile();
    step2 = tmpfile();
    if (step1 == NULL || step2 == NULL) {
        printf("Error: cannot create temp file\n");
        fclose(input);
        if (step1 != NULL)
            fclose(step1);
        if (step2 != NULL)
            fclose(step2);
        return EXIT_IO;
    }

    include_files(input, step1, argv[1]);
    fclose(input);

    rewind(step1);
    remove_comments(step1, step2);
    fclose(step1);

    rewind(step2);
    output = fopen(out_name, "w");
    if (output == NULL) {
        printf("Error: cannot write %s\n", out_name);
        fclose(step2);
        return EXIT_IO;
    }
    process_macros(step2, output);
    fclose(step2);
    fclose(output);

    printf("Created %s\n", out_name);
    return 0;
}
