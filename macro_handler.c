/* macro_handler.c -- store #define lines and replace the names used later.
 *
 * The file is handled one line at a time:
 *
 *   1. if the line is a #define, store the macro and do not copy the line;
 *   2. if the line is another # line, copy it unchanged;
 *   3. otherwise copy the line while replacing macro names.
 *
 * Comments are already gone (that was stage 1), so we only have to look out
 * for "strings" and 'characters' to leave those alone.
 */
#include <stdio.h>
#include <string.h>
#include "macro_handler.h"
#include "util.h"

#define MAX_MACROS 2000 /* more than enough for one project file */
#define MAX_PARAMS 20   /* parameters a single macro may have   */
#define NAME_MAX 128    /* longest macro or parameter name      */
#define VALUE_MAX 1024  /* longest macro body                   */
#define MAX_EXPAND_DEPTH 64 /* safety net for macro chains      */

/* One macro: its name, its parameters and its body. */
typedef struct {
    char name[NAME_MAX];
    char value[VALUE_MAX];
    int param_count;
    char params[MAX_PARAMS][NAME_MAX];
    /* The arguments of the use we are expanding right now.  Keeping them
     * here (instead of passing them around) means a macro that is expanded
     * inside another macro always finds its own arguments. */
    char args[MAX_PARAMS][VALUE_MAX];
} Macro;

static Macro macros[MAX_MACROS];
static int macro_count = 0;

/* ---------------------------------------------------------------------- */
/* the macro table                                                        */
/* ---------------------------------------------------------------------- */

static int find_macro(const char *name) {
    int m;
    /* from the end, so a macro that was redefined uses its newest value */
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

/* ---------------------------------------------------------------------- */
/* reading #define lines                                                  */
/* ---------------------------------------------------------------------- */

/* Store "#define NAME value" or "#define NAME(a, b) value".
 * Returns 1 when the macro was stored, 0 when the line is not one we can
 * use (for example "#define NAME(x) ..." spanning two lines). */
static int store_macro(const char *line, int after_define) {
    Macro m;
    char word[NAME_MAX];
    int i = after_define;
    int p;

    memset(&m, 0, sizeof(m));

    while (is_blank(line[i]))
        i++;

    /* the macro name */
    i = read_word(line, i, word, sizeof(word));
    if (word[0] == '\0')
        return 0;
    copy_text(m.name, sizeof(m.name), word);

    /* an opening bracket must follow the name directly: NAME(a, b) */
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
                i++; /* skip commas and spaces */
            }
        }
        if (line[i] == ')')
            i++;
    }

    while (is_blank(line[i]))
        i++;

    /* the rest of the line is the value */
    copy_text(m.value, sizeof(m.value), &line[i]);
    trim_right(m.value);
    if (m.value[0] == '\0')
        copy_text(m.value, sizeof(m.value), ""); /* e.g. "#define DEBUG" */

    /* a macro with a value that ends in a backslash continues on the next
     * line, which we do not support; keep the old definition instead of
     * storing something broken. */
    if (m.value[0] != '\0' && m.value[strlen(m.value) - 1] == '\\')
        return 0;

    /* redefine in place, or add a new one */
    p = find_macro(m.name);
    if (p >= 0) {
        macros[p] = m;
        return 1;
    }
    if (macro_count >= MAX_MACROS) {
        printf("Warning: more than %d macros, ignoring %s\n", MAX_MACROS,
               m.name);
        return 1; /* still hide the line from the output */
    }
    macros[macro_count] = m;
    macro_count++;
    return 1;
}

/* ---------------------------------------------------------------------- */
/* reading the arguments of a call: ADD(1, 2 + 3)                         */
/* ---------------------------------------------------------------------- */

/* text[i] is the '('.  Split the arguments at the commas that are not
 * inside another pair of brackets, splitting arg_text into pieces.
 * Fills args[] and returns how many arguments were found. */
static int read_args(const char *text, int i, char args[][VALUE_MAX],
                     int max_args) {
    int count = 0;
    int depth = 0;
    int start;

    i++; /* step over the '(' */
    start = i;

    for (;;) {
        char ch = text[i];

        if (ch == '\0' || ch == '\n')
            break; /* no closing bracket, give up */
        if (ch == '"' || ch == '\'') {
            i = skip_quoted(text, i);
            continue;
        }
        if (ch == '(') {
            depth++;
        } else if (ch == ')') {
            if (depth == 0) {
                /* the last argument ends here */
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

    /* unterminated call: treat what we have as one argument */
    if (count < max_args && start < i) {
        args[count][0] = '\0';
        add_slice(args[count], VALUE_MAX, text, start, i);
        trim_left(args[count]);
        trim_right(args[count]);
        count++;
    }
    return count;
}

/* Return the position just after the ')' that closes the '(' at text[i]. */
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

/* ---------------------------------------------------------------------- */
/* replacing macro names in one line                                      */
/* ---------------------------------------------------------------------- */

static void replace_line(const char *line, char *out, int max);

/* Recursion guard: a macro argument can itself contain a macro call, and
 * that call may read arguments of its own. */
static int expand_level = 0;

/* Expand the macros inside every argument.  The results go to result[][];
 * args[][] is only read.  The two must not be the same array, because
 * replace_line() may change the macro table while it works.
 * Example: MAX(1, MAX(2, 3)) -> the second argument becomes
 *          ((2) > (3) ? (2) : (3)) before MAX is used. */
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

/* Read the arguments of a macro call that starts at text[i] ('('), expand
 * them and store them with the macro, ready for expand_use().
 * Returns the position after the call, or -1 when the number of arguments
 * is wrong (the caller then copies the call unchanged). */
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

/* Expand one macro use and write the result to out.
 *
 * The arguments of this use are read from macros[m].args once, at the
 * start.  Everything else works from that private copy, so expanding a
 * macro inside the value (possibly the same macro again) cannot disturb
 * what we are in the middle of doing.
 *
 * "active" holds the macros we are busy expanding.  If a macro contains
 * itself (#define A A) or two macros use each other, we stop and copy the
 * plain name instead of looping forever. */
static void expand_use(int m, char *out, int max, int active[], int depth) {
    char word[NAME_MAX];
    char my_args[MAX_PARAMS][VALUE_MAX];
    const char *value = macros[m].value;
    int i;
    int a;

    for (a = 0; a < macros[m].param_count; a++)
        copy_text(my_args[a], VALUE_MAX, macros[m].args[a]);

    active[m] = 1;

    for (i = 0; value[i] != '\0';) {
        char ch = value[i];

        if (ch == '"' || ch == '\'') {
            i = copy_quoted(value, i, out, max); /* strings stay as written */
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
                add_text(out, max, word); /* plain text, or a loop: keep it */
                continue;
            }

            if (macros[other].param_count > 0) {
                int end;

                if (value[i] != '(') {
                    add_text(out, max, word); /* a name without brackets */
                    continue;
                }
                end = prepare_args(other, value, i);
                if (end < 0) {
                    add_text(out, max, word); /* wrong argument count */
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

/* Replace every macro name in line and write the result to out. */
static void replace_line(const char *line, char *out, int max) {
    char word[NAME_MAX];
    char value[VALUE_MAX];
    int active[MAX_MACROS]; /* no macro is being expanded yet */
    int i;

    for (i = 0; i < MAX_MACROS; i++)
        active[i] = 0;

    i = 0;
    out[0] = '\0';
    while (line[i] != '\0') {
        if (line[i] == '"' || line[i] == '\'') {
            i = copy_quoted(line, i, out, max); /* never touch strings */
        } else if (!is_name_char(line[i])) {
            add_char(out, max, line[i]);
            i++;
        } else {
            int m;

            i = read_word(line, i, word, sizeof(word));
            m = find_macro(word);
            if (m < 0) {
                add_text(out, max, word); /* not a macro, keep the name */
            } else if (macros[m].param_count == 0) {
                expand_use(m, out, max, active, 0);
            } else if (line[i] == '(') {
                int end = prepare_args(m, line, i);

                if (end < 0) {
                    add_text(out, max, word); /* wrong argument count */
                } else {
                    value[0] = '\0';
                    expand_use(m, value, sizeof(value), active, 0);
                    add_text(out, max, value);
                    i = end;
                }
            } else {
                add_text(out, max, word); /* macro with parameters, but no
                                             brackets after the name */
            }
        }
    }
}


/* ---------------------------------------------------------------------- */
/* the whole file                                                         */
/* ---------------------------------------------------------------------- */

void process_macros(FILE *in, FILE *out) {
    char line[MAX_LINE];
    char directive[64];
    char out_line[MAX_LINE];
    int in_comment = 0;

    while (fgets(line, sizeof(line), in) != NULL) {
        int after = read_directive(line, directive, sizeof(directive));

        /* A line that only looks like a directive because it sits inside a
         * block comment must not be treated as one.  This can happen when a
         * header is pasted in as one very long line. */
        if (in_comment || after < 0) {
            in_comment = comment_state_after(line, in_comment);
            replace_line(line, out_line, sizeof(out_line));
            fputs(out_line, out);
            continue;
        }

        if (strcmp(directive, "define") == 0) {
            if (!store_macro(line, after)) {
                /* something we cannot store: keep the line so the reader
                 * can see that it was left alone */
                fputs(line, out);
            }
        } else {
            fputs(line, out); /* #include, #ifdef, #pragma, ... */
        }
    }
}
