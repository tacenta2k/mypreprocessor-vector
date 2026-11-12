// pastes in #include files, recursively
#define _POSIX_C_SOURCE 200809L // for popen
#include <stdio.h>
#include <string.h>
#include "file_inclusion.h"
#include "util.h"

#define MAX_FILES 100

// where <...> headers are looked for
#define MAX_SYSTEM_DIRS 4
static char system_dirs[MAX_SYSTEM_DIRS][MAX_PATH] = {
    "/usr/include", "/usr/include/x86_64-linux-gnu", "/usr/local/include",
    "" // filled by find_gcc_dir
};
static int system_dir_count = 3;

// gcc headers like stddef.h live in a versioned folder, ask gcc
static void find_gcc_dir(void) {
    FILE *p = popen("gcc -print-file-name=include 2>/dev/null", "r");

    if (p == NULL)
        return;
    if (fgets(system_dirs[3], MAX_PATH, p) != NULL) {
        trim_right(system_dirs[3]);
        if (system_dirs[3][0] == '/')
            system_dir_count = 4;
    }
    pclose(p);
}

// files already pasted, so a header is included only once
static char seen[MAX_FILES][MAX_PATH];
static int seen_count = 0;

static int already_pasted(const char *path) {
    int i;
    for (i = 0; i < seen_count; i++) {
        if (strcmp(seen[i], path) == 0)
            return 1;
    }
    return 0;
}

static void remember_pasted(const char *path) {
    if (seen_count < MAX_FILES)
        copy_text(seen[seen_count++], MAX_PATH, path);
}

// read name from #include <name> or "name"
static int read_include_name(const char *line, int i, char *name, int max,
                             int *is_system) {
    char close;

    while (is_blank(line[i]))
        i++;

    if (line[i] == '<') {
        close = '>';
        *is_system = 1;
    } else if (line[i] == '"') {
        close = '"';
        *is_system = 0;
    } else {
        return 0; // not a form we know
    }

    i++;
    name[0] = '\0';
    while (line[i] != close && line[i] != '\n' && line[i] != '\0') {
        add_char(name, max, line[i]);
        i++;
    }
    return name[0] != '\0';
}

// copy a stream, expanding every #include in it
static void copy_with_includes(FILE *in, FILE *out, const char *dir) {
    char line[MAX_LINE];
    char directive[64];
    char name[MAX_PATH];
    char path[MAX_PATH];

    while (fgets(line, sizeof(line), in) != NULL) {
        int after = read_directive(line, directive, sizeof(directive));
        int is_system = 0;
        int found = 0;
        int i;

        if (after < 0 || strcmp(directive, "include") != 0 ||
            !read_include_name(line, after, name, sizeof(name), &is_system)) {
            fputs(line, out);
            continue;
        }

        if (is_system) {
            for (i = 0; i < system_dir_count && !found; i++) {
                FILE *header;

                join_path(system_dirs[i], name, path, sizeof(path));
                if (already_pasted(path)) {
                    found = 1;
                    break;
                }
                header = fopen(path, "r");
                if (header != NULL) {
                    found = 1;
                    remember_pasted(path);
                    copy_with_includes(header, out, system_dirs[i]);
                    fclose(header);
                }
            }
        } else {
            FILE *header;

            join_path(dir, name, path, sizeof(path));
            if (already_pasted(path)) {
                found = 1;
            } else {
                header = fopen(path, "r");
                if (header != NULL) {
                    char header_dir[MAX_PATH];

                    found = 1;
                    remember_pasted(path);
                    dir_of(path, header_dir, sizeof(header_dir));
                    copy_with_includes(header, out, header_dir);
                    fclose(header);
                }
            }
        }

        if (!found) {
            printf("Warning: cannot open %s\n", name);
            fputs(line, out); // keep the line so the gap is visible
        } else {
            fputc('\n', out);
        }
    }
}

int include_files(FILE *in, FILE *out, const char *path) {
    char dir[MAX_PATH];

    find_gcc_dir();
    dir_of(path, dir, sizeof(dir));
    remember_pasted(path); // so a file including itself is skipped
    copy_with_includes(in, out, dir);
    return 0;
}
