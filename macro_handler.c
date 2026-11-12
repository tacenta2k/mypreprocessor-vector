#include <stdio.h>
#include <string.h>
#include "macro_handler.h"

#define MAX_MACROS 200
#define MAX_PARAMS 10
#define NAME_SIZE 100
#define LINE_SIZE 1000

char macro_name[MAX_MACROS][NAME_SIZE];
char macro_value[MAX_MACROS][LINE_SIZE];
char param_name[MAX_MACROS][MAX_PARAMS][NAME_SIZE];
int param_count[MAX_MACROS];
int macro_count = 0;

int is_name_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void add_char(char str[], char c) {
    int len = strlen(str);
    if (len < LINE_SIZE - 1) {
        str[len] = c;
        str[len + 1] = '\0';
    }
}

void add_text(char str[], char text[]) {
    if (strlen(str) + strlen(text) < LINE_SIZE)
        strcat(str, text);
}

void remove_end_spaces(char str[]) {
    int len = strlen(str);
    while (len > 0 && is_space(str[len - 1]))
        len--;
    str[len] = '\0';
}

// reads the name starting at text[i] into word, returns the index after it
int read_word(char text[], int i, char word[]) {
    int k = 0;
    while (is_name_char(text[i]) && k < NAME_SIZE - 1) {
        word[k] = text[i];
        k++;
        i++;
    }
    word[k] = '\0';
    return i;
}

// copies a string like "abc" or a char like 'a' to out as it is
int copy_quoted(char text[], int i, char out[]) {
    char quote = text[i];

    add_char(out, text[i]);
    i++;
    while (text[i] != quote && text[i] != '\0') {
        if (text[i] == '\\' && text[i + 1] != '\0') {
            add_char(out, text[i]);
            i++;
        }
        add_char(out, text[i]);
        i++;
    }
    if (text[i] == quote) {
        add_char(out, text[i]);
        i++;
    }
    return i;
}

int find_macro(char word[]) {
    int m;
    // search from the end, so a redefined macro uses its new value
    for (m = macro_count - 1; m >= 0; m--) {
        if (strcmp(macro_name[m], word) == 0)
            return m;
    }
    return -1;
}

int find_param(int m, char word[]) {
    int p;
    for (p = 0; p < param_count[m]; p++) {
        if (strcmp(param_name[m][p], word) == 0)
            return p;
    }
    return -1;
}

// stores a line like #define PI 3.14 or #define ADD(a, b) ((a) + (b))
void store_macro(char line[]) {
    int m, p;
    int i = 7;

    if (macro_count == MAX_MACROS) {
        printf("Warning: too many macros\n");
        return;
    }
    m = macro_count;
    macro_count++;

    while (is_space(line[i]))
        i++;
    i = read_word(line, i, macro_name[m]);

    p = 0;
    if (line[i] == '(') {
        i++;
        while (line[i] != ')' && line[i] != '\0') {
            if (is_name_char(line[i]) && p < MAX_PARAMS) {
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

    while (is_space(line[i]))
        i++;
    strcpy(macro_value[m], &line[i]);
    remove_end_spaces(macro_value[m]);
}

// reads the arguments of a call like ADD(10, 20), text[i] is the '('
int read_args(char text[], int i, char args[][LINE_SIZE]) {
    int n = 0, depth = 0, p;

    for (p = 0; p < MAX_PARAMS; p++)
        args[p][0] = '\0';

    i++;
    while (text[i] != '\0') {
        if (text[i] == ')' && depth == 0) {
            remove_end_spaces(args[n]);
            return i + 1;
        } else if (text[i] == ',' && depth == 0) {
            remove_end_spaces(args[n]);
            if (n < MAX_PARAMS - 1)
                n++;
            i++;
        } else if (text[i] == ' ' && args[n][0] == '\0') {
            i++;
        } else if (text[i] == '"' || text[i] == '\'') {
            i = copy_quoted(text, i, args[n]);
        } else {
            if (text[i] == '(')
                depth++;
            if (text[i] == ')')
                depth--;
            add_char(args[n], text[i]);
            i++;
        }
    }
    return i;
}

// copies the value of macro m to out, with the parameters replaced by the arguments
void fill_args(int m, char args[][LINE_SIZE], char out[]) {
    char word[NAME_SIZE];
    int i = 0, p;

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

void replace_line(char line[], char result[]) {
    char word[NAME_SIZE];
    char args[MAX_PARAMS][LINE_SIZE];
    int i = 0, m;

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
                i = read_args(line, i, args);
                fill_args(m, args, result);
            } else {
                add_text(result, word);
            }
        } else {
            add_char(result, line[i]);
            i++;
        }
    }
}

void process_macros(char in_name[], char out_name[]) {
    FILE *in = fopen(in_name, "r");
    FILE *out = fopen(out_name, "w");
    char line[LINE_SIZE], result[LINE_SIZE];
    int pass;

    while (fgets(line, LINE_SIZE, in) != NULL) {
        if (strncmp(line, "#define", 7) == 0) {
            store_macro(line);
        } else if (line[0] == '#') {
            fputs(line, out);
        } else {
            // repeat until nothing changes, for a macro used inside another macro
            for (pass = 0; pass < 10; pass++) {
                replace_line(line, result);
                if (strcmp(line, result) == 0)
                    break;
                strcpy(line, result);
            }
            fputs(line, out);
        }
    }

    fclose(in);
    fclose(out);
}
