// stores #define lines and replaces the names later
#include <stdio.h>
#include <string.h>
#include "macro_handler.h"
#include "util.h"

#define MAX_MACROS 2000
#define MAX_PARAMS 20
#define NAME_MAX 128
#define VALUE_MAX 1024
#define MAX_EXPAND_DEPTH 64

typedef struct {
    char name[NAME_MAX];
    char value[VALUE_MAX];
    int param_count;
    char params[MAX_PARAMS][NAME_MAX];
    char args[MAX_PARAMS][VALUE_MAX]; // args of the use being expanded
} Macro;

static Macro macros[MAX_MACROS];
static int macro_count = 0;

// newest first, so a redefined macro wins
static int find_macro(const char *name) {
    int m;
    for (m = macro_count - 1; m >= 0; m--) {
        if (strcmp(macros[m].name, name) == 0)
            return m;
    }
    return -1;
}

static int find_param(int m, const char *name) {
    int p;
    for (p = 0; p < macros[m].param_count; p++) {
        if (strcmp(macros[m].params[p], name) == 0)
            return p;
    }
    return -1;
}

// store #define NAME value or #define NAME(a, b) value
// returns 1 if the line was handled
static int store_macro(const char *line, int after_define) {
    Macro m;
    char word[NAME_MAX];
    int i = after_define;
    int p;

    memset(&m, 0, sizeof(m));

    while (is_blank(line[i]))
        i++;

    i = read_word(line, i, word, sizeof(word));
    if (word[0] == '\0')
        return 0;
    copy_text(m.name, sizeof(m.name), word);

    // no space before ( means it takes params
    if (line[i] == '(') {
        i++;
        while (line[i] != ')' && line[i] != '\n' && line[i] != '\0') {
            if (is_name_char(line[i])) {
                i = read_word(line, i, word, sizeof(word));
                if (m.param_count < MAX_PARAMS) {
                    copy_text(m.params[m.param_count], NAME_MAX, word);
                    m.param_count++;
                }
            } else {
                i++; // skip , and spaces
            }
        }
        if (line[i] == ')')
            i++;
    }

    while (is_blank(line[i]))
        i++;

    // rest of the line is the value
    copy_text(m.value, sizeof(m.value), &line[i]);
    trim_right(m.value);

    // value continues on the next line, not supported
    if (m.value[0] != '\0' && m.value[strlen(m.value) - 1] == '\\')
        return 0;

    p = find_macro(m.name);
    if (p >= 0) {
        macros[p] = m; // redefined
        return 1;
    }
    if (macro_count >= MAX_MACROS) {
        printf("Warning: too many macros, skipping %s\n", m.name);
        return 1;
    }
    macros[macro_count] = m;
    macro_count++;
    return 1;
}

// split ADD(1, 2+3) into "1" and "2+3"
// text[i] is the (, returns how many args
static int read_args(const char *text, int i, char args[][VALUE_MAX],
                     int max_args) {
    int count = 0;
    int depth = 0;
    int start;

    i++;
    start = i;

    for (;;) {
        char ch = text[i];

        if (ch == '\0' || ch == '\n')
            break;
        if (ch == '"' || ch == '\'') {
            i = skip_quoted(text, i);
            continue;
        }
        if (ch == '(') {
            depth++;
        } else if (ch == ')') {
            if (depth == 0) {
                if (count < max_args) {
                    args[count][0] = '\0';
                    add_slice(args[count], VALUE_MAX, text, start, i);
                    trim_left(args[count]);
                    trim_right(args[count]);
                    count++;
                }
                return count;
            }
            depth--;
        } else if (ch == ',' && depth == 0) {
            if (count < max_args) {
                args[count][0] = '\0';
                add_slice(args[count], VALUE_MAX, text, start, i);
                trim_left(args[count]);
                trim_right(args[count]);
                count++;
            }
            start = i + 1;
        }
        i++;
    }

    // no closing )
    if (count < max_args && start < i) {
        args[count][0] = '\0';
        add_slice(args[count], VALUE_MAX, text, start, i);
        trim_left(args[count]);
        trim_right(args[count]);
        count++;
    }
    return count;
}

// position after the ) that closes the ( at text[i]
static int skip_call(const char *text, int i) {
    int depth = 0;

    for (; text[i] != '\0' && text[i] != '\n'; i++) {
        if (text[i] == '"' || text[i] == '\'') {
            i = skip_quoted(text, i) - 1;
        } else if (text[i] == '(') {
            depth++;
        } else if (text[i] == ')') {
            depth--;
            if (depth == 0)
                return i + 1;
        }
    }
    return i;
}

static void replace_line(const char *line, char *out, int max);

// guard for args that contain macro calls
static int expand_level = 0;

// expand macros inside args, result must not be the same array
static void expand_args(char args[][VALUE_MAX], int argc,
                        char result[][VALUE_MAX]) {
    char tmp[VALUE_MAX];
    int a;

    for (a = 0; a < argc; a++) {
        if (expand_level >= MAX_EXPAND_DEPTH) {
            copy_text(result[a], VALUE_MAX, args[a]);
            continue;
        }
        expand_level++;
        replace_line(args[a], tmp, sizeof(tmp));
        expand_level--;
        copy_text(result[a], VALUE_MAX, tmp);
    }
}

// read and expand the args of a call, store them in macros[m].args
// returns pos after the call, -1 if the arg count is wrong
static int prepare_args(int m, const char *text, int i) {
    char raw[MAX_PARAMS][VALUE_MAX];
    char expanded[MAX_PARAMS][VALUE_MAX];
    int argc = read_args(text, i, raw, MAX_PARAMS);
    int a;

    if (argc != macros[m].param_count)
        return -1;

    expand_args(raw, argc, expanded);
    for (a = 0; a < argc; a++)
        copy_text(macros[m].args[a], VALUE_MAX, expanded[a]);
    return skip_call(text, i);
}

// write the expanded value of macro m to out
// active[] stops #define A A from looping
static void expand_use(int m, char *out, int max, int active[], int depth) {
    char word[NAME_MAX];
    char my_args[MAX_PARAMS][VALUE_MAX];
    const char *value = macros[m].value;
    int i;
    int a;

    // own copy, nested calls would overwrite macros[m].args
    for (a = 0; a < macros[m].param_count; a++)
        copy_text(my_args[a], VALUE_MAX, macros[m].args[a]);

    active[m] = 1;

    for (i = 0; value[i] != '\0';) {
        char ch = value[i];

        if (ch == '"' || ch == '\'') {
            i = copy_quoted(value, i, out, max);
        } else if (!is_name_char(ch)) {
            add_char(out, max, ch);
            i++;
        } else {
            int p;
            int other;

            i = read_word(value, i, word, sizeof(word));

            p = find_param(m, word);
            if (p >= 0) {
                add_text(out, max, my_args[p]);
                continue;
            }

            other = find_macro(word);
            if (other < 0 || depth >= MAX_EXPAND_DEPTH || active[other]) {
                add_text(out, max, word);
                continue;
            }

            if (macros[other].param_count > 0) {
                int end;

                if (value[i] != '(') {
                    add_text(out, max, word);
                    continue;
                }
                end = prepare_args(other, value, i);
                if (end < 0) {
                    add_text(out, max, word); // wrong arg count
                    i = skip_call(value, i);
                    continue;
                }
                i = end;
            }
            expand_use(other, out, max, active, depth + 1);
        }
    }

    active[m] = 0;
}

// replace all macro names in one line
static void replace_line(const char *line, char *out, int max) {
    char word[NAME_MAX];
    char value[VALUE_MAX];
    int active[MAX_MACROS];
    int i;

    for (i = 0; i < MAX_MACROS; i++)
        active[i] = 0;

    i = 0;
    out[0] = '\0';
    while (line[i] != '\0') {
        if (line[i] == '"' || line[i] == '\'') {
            i = copy_quoted(line, i, out, max);
        } else if (!is_name_char(line[i])) {
            add_char(out, max, line[i]);
            i++;
        } else {
            int m;

            i = read_word(line, i, word, sizeof(word));
            m = find_macro(word);
            if (m < 0) {
                add_text(out, max, word);
            } else if (macros[m].param_count == 0) {
                expand_use(m, out, max, active, 0);
            } else if (line[i] == '(') {
                int end = prepare_args(m, line, i);

                if (end < 0) {
                    add_text(out, max, word); // wrong arg count
                } else {
                    value[0] = '\0';
                    expand_use(m, value, sizeof(value), active, 0);
                    add_text(out, max, value);
                    i = end;
                }
            } else {
                add_text(out, max, word); // needs ( after the name
            }
        }
    }
}

void process_macros(FILE *in, FILE *out) {
    char line[MAX_LINE];
    char directive[64];
    char out_line[MAX_LINE];
    int in_comment = 0;

    while (fgets(line, sizeof(line), in) != NULL) {
        int after = read_directive(line, directive, sizeof(directive));

        // skip lines that only look like a directive, they are in a comment
        if (in_comment || after < 0) {
            in_comment = comment_state_after(line, in_comment);
            replace_line(line, out_line, sizeof(out_line));
            fputs(out_line, out);
            continue;
        }

        if (strcmp(directive, "define") == 0) {
            if (!store_macro(line, after))
                fputs(line, out); // not supported, keep it visible
        } else {
            fputs(line, out); // #include, #ifdef, ...
        }
    }
}
