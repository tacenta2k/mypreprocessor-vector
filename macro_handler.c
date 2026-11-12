// stores #define, replaces names
#include <stdio.h>
#include <string.h>
#include "macro_handler.h"

#define MAX_MACROS 200
#define MAX_PARAMS 10
#define NAMESIZE 100
#define LINESIZE 500

char macro_name[MAX_MACROS][NAMESIZE];
char macro_value[MAX_MACROS][LINESIZE];
char param_name[MAX_MACROS][MAX_PARAMS][NAMESIZE];
int param_count[MAX_MACROS];
int macro_count = 0;

// a-z A-Z 0-9 _
int is_name_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// add char
void add_char(char s[], char c) {
    int n = strlen(s);
    if (n < LINESIZE - 1) {
        s[n] = c;
        s[n + 1] = '\0';
    }
}

// add text
void add_text(char s[], char t[]) {
    int n = strlen(s);
    int i = 0;
    while (t[i] != '\0' && n < LINESIZE - 1) {
        s[n] = t[i];
        n++;
        i++;
    }
    s[n] = '\0';
}

// trim end
void trim_end(char s[]) {
    int n = strlen(s);
    while (n > 0 && is_space(s[n - 1]))
        n--;
    s[n] = '\0';
}

// read name
int read_word(char s[], int i, char word[]) {
    int k = 0;
    while (is_name_char(s[i]) && k < NAMESIZE - 1) {
        word[k] = s[i];
        k++;
        i++;
    }
    word[k] = '\0';
    return i;
}

// find closing )
int skip_call(char s[], int i) {
    int depth = 0;
    while (s[i] != '\0') {
        if (s[i] == '"' || s[i] == '\'') {
            int q = s[i];
            i++;
            while (s[i] != '\0' && s[i] != q) {
                if (s[i] == '\\')
                    i++;
                i++;
            }
            if (s[i] == q)
                i++;
            continue;
        }
        if (s[i] == '(')
            depth++;
        if (s[i] == ')') {
            depth--;
            if (depth == 0)
                return i + 1;
        }
        i++;
    }
    return i;
}

// copy quoted
int copy_quoted(char s[], int i, char out[]) {
    int q = s[i];
    add_char(out, s[i]);
    i++;
    while (s[i] != '\0' && s[i] != q) {
        if (s[i] == '\\') {
            add_char(out, s[i]);
            i++;
        }
        add_char(out, s[i]);
        i++;
    }
    if (s[i] == q) {
        add_char(out, s[i]);
        i++;
    }
    return i;
}

// find macro
int find_macro(char word[]) {
    int m;
    for (m = macro_count - 1; m >= 0; m--) {
        if (strcmp(macro_name[m], word) == 0)
            return m;
    }
    return -1;
}

// find param number
int find_param(int m, char word[]) {
    int p;
    for (p = 0; p < param_count[m]; p++) {
        if (strcmp(param_name[m][p], word) == 0)
            return p;
    }
    return -1;
}

// store one #define
void store_macro(char line[]) {
    int m, p, i;
    char word[NAMESIZE];

    if (macro_count == MAX_MACROS)
        return;

    // skip to name
    i = 0;
    while (is_space(line[i]))
        i++;
    i = i + 7;
    while (is_space(line[i]))
        i++;

    i = read_word(line, i, word);
    if (word[0] == '\0')
        return;

    // redefine?
    m = find_macro(word);
    if (m == -1) {
        m = macro_count;
        macro_count++;
    }
    strcpy(macro_name[m], word);

    // ( right after name = has params
    p = 0;
    if (line[i] == '(') {
        i++;
        while (line[i] != ')' && line[i] != '\0' && p < MAX_PARAMS) {
            if (is_name_char(line[i])) {
                i = read_word(line, i, param_name[m][p]);
                p++;
            } else {
                i++;
            }
        }
        if (line[i] == ')')
            i++;
    }
    param_count[m] = p;

    // value
    while (is_space(line[i]))
        i++;
    strcpy(macro_value[m], &line[i]);
    trim_end(macro_value[m]);
}

// copy one arg
void copy_arg(char args[], char s[], int start, int end) {
    int k = 0;

    while (start < end && (s[start] == ' ' || s[start] == '\t'))
        start++;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;

    while (start < end && k < LINESIZE - 1) {
        args[k] = s[start];
        k++;
        start++;
    }
    args[k] = '\0';
}

// read args, s[i] is (
// returns pos, sets *count
int read_args(char s[], int i, char args[][LINESIZE], int *count) {
    int n = 0;
    int depth = 0;
    int start;
    int k;

    for (k = 0; k < MAX_PARAMS; k++)
        args[k][0] = '\0';

    i++;
    start = i;

    while (s[i] != '\0') {
        if (s[i] == '"' || s[i] == '\'') {
            int q = s[i];
            i++;
            while (s[i] != '\0' && s[i] != q) {
                if (s[i] == '\\')
                    i++;
                i++;
            }
            if (s[i] == q)
                i++;
            continue;
        }

        if (s[i] == '(') {
            depth++;
        } else if (s[i] == ')' && depth == 0) {
            copy_arg(args[n], s, start, i);
            *count = n + 1;
            return i + 1;
        } else if (s[i] == ')') {
            depth--;
        } else if (s[i] == ',' && depth == 0) {
            copy_arg(args[n], s, start, i);
            if (n < MAX_PARAMS - 1)
                n++;
            start = i + 1;
        }
        i++;
    }

    *count = n + 1;
    return i;
}

// put args in value
void fill_value(int m, char args[][LINESIZE], char out[]) {
    char word[NAMESIZE];
    int i = 0;
    int p;

    while (macro_value[m][i] != '\0') {
        if (macro_value[m][i] == '"' || macro_value[m][i] == '\'') {
            i = copy_quoted(macro_value[m], i, out);
        } else if (is_name_char(macro_value[m][i])) {
            i = read_word(macro_value[m], i, word);
            p = find_param(m, word);
            if (p == -1)
                add_text(out, word);
            else
                add_text(out, args[p]);
        } else {
            add_char(out, macro_value[m][i]);
            i++;
        }
    }
}

// replace macros in line
void replace_line(char line[], char result[]) {
    char word[NAMESIZE];
    char args[MAX_PARAMS][LINESIZE];
    char value[LINESIZE];
    int i = 0;
    int m;
    int argc;
    int end;

    result[0] = '\0';

    while (line[i] != '\0') {
        if (line[i] == '"' || line[i] == '\'') {
            i = copy_quoted(line, i, result);
        } else if (is_name_char(line[i])) {
            i = read_word(line, i, word);
            m = find_macro(word);

            if (m == -1) {
                add_text(result, word);
            } else if (param_count[m] == 0) {
                add_text(result, macro_value[m]);
            } else if (line[i] == '(') {
                end = read_args(line, i, args, &argc);

                if (param_count[m] == argc) {
                    value[0] = '\0';
                    fill_value(m, args, value);
                    add_text(result, value);
                    i = end;
                } else {
                    add_text(result, word);
                    // keep ( )
                }
            } else {
                add_text(result, word);
            }
        } else {
            add_char(result, line[i]);
            i++;
        }
    }
}

void process_macros(char in[], char out[]) {
    FILE *fp = fopen(in, "r");
    FILE *fq = fopen(out, "w");
    char line[LINESIZE];
    char result[LINESIZE];
    int pass;

    if (fp == NULL || fq == NULL)
        return;

    while (fgets(line, LINESIZE, fp) != NULL) {
        int start = 0;

        // skip spaces
        while (line[start] == ' ' || line[start] == '\t')
            start++;

        if (strncmp(&line[start], "#define", 7) == 0) {
            store_macro(line);
        } else if (line[start] == '#') {
            fputs(line, fq);
        } else {
            // again, value may have macros
            for (pass = 0; pass < 10; pass++) {
                replace_line(line, result);
                if (strcmp(line, result) == 0)
                    break;
                strcpy(line, result);
            }
            fputs(line, fq);
        }
    }

    fclose(fp);
    fclose(fq);
}
