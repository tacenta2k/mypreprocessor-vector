/* main.c -- run the three stages of the preprocessor on one .c file.
 *
 *   test.c  --include-->  --remove comments-->  --expand macros-->  test.i
 *
 * The three stages are chained in memory with tmpfile(), so nothing is
 * written to disk except the final .i file and no temporary files are left
 * behind when something goes wrong.
 *
 * Usage:  ./mypreprocessor test.c
 *         ./mypreprocessor            (prints a short help text)
 */
#include <stdio.h>
#include <string.h>
#include "comment_removal.h"
#include "file_inclusion.h"
#include "macro_handler.h"
#include "util.h"

#define EXIT_USAGE 1 /* the command line was wrong        */
#define EXIT_INPUT 2 /* the input file could not be read  */
#define EXIT_IO 3    /* an output file could not be made  */

static void print_usage(const char *program) {
    printf("Usage: %s file.c\n", program);
    printf("\n");
    printf("Runs the three preprocessor steps on file.c:\n");
    printf("  1. paste in the files named by #include\n");
    printf("  2. remove the comments\n");
    printf("  3. replace the names created by #define\n");
    printf("\n");
    printf("The result is written to file.i next to the input file.\n");
    printf("Example: %s test.c   creates test.i\n", program);
}

/* Check that the name ends in ".c".  Returns 1 for yes. */
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
        printf("Error: please give only one file name\n\n");
        print_usage(argv[0]);
        return EXIT_USAGE;
    }
    if (!has_c_extension(argv[1])) {
        printf("Error: %s is not a .c file\n", argv[1]);
        return EXIT_USAGE;
    }
    if (strlen(argv[1]) >= MAX_PATH) {
        printf("Error: the file name is too long (limit: %d characters)\n",
               MAX_PATH - 1);
        return EXIT_USAGE;
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Error: cannot open %s\n", argv[1]);
        printf("Check that the file exists and that the name is spelled "
               "correctly.\n");
        return EXIT_INPUT;
    }

    /* test.c -> test.i, keeping the folder: src/test.c -> src/test.i */
    copy_text(out_name, sizeof(out_name), argv[1]);
    len = (int)strlen(out_name);
    out_name[len - 1] = 'i';

    /* The three stages talk to each other through temporary streams that
     * disappear when they are closed. */
    step1 = tmpfile();
    step2 = tmpfile();
    if (step1 == NULL || step2 == NULL) {
        printf("Error: cannot create a temporary file\n");
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
        printf("Error: cannot create %s\n", out_name);
        printf("Check that you are allowed to write in that folder.\n");
        fclose(step2);
        return EXIT_IO;
    }
    process_macros(step2, output);
    fclose(step2);
    fclose(output);

    printf("Created %s\n", out_name);
    return 0;
}
