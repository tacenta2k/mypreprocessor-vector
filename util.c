/* util.c -- the small helpers declared in util.h. */
#include <string.h>
#include "util.h"

/* ---------------------------------------------------------------------- */
/* building strings safely                                                */
/* ---------------------------------------------------------------------- */

/* Add one character at the end of dest, if there is still room. */
void add_char(char *dest, int max, char c) {
    int len = (int)strlen(dest);
    if (len < max - 1) {
        dest[len] = c;
        dest[len + 1] = '\0';
    }
}

/* Add text at the end of dest, copying only what fits. */
void add_text(char *dest, int max, const char *text) {
    int len = (int)strlen(dest);
    int i = 0;
    while (text[i] != '\0' && len < max - 1) {
        dest[len] = text[i];
        len++;
        i++;
    }
    dest[len] = '\0';
}

/* Add src[from .. to-1] at the end of dest.  Used to copy a piece of a
 * line exactly as it was written. */
void add_slice(char *dest, int max, const char *src, int from, int to) {
    int len = (int)strlen(dest);
    while (from < to && src[from] != '\0' && len < max - 1) {
        dest[len] = src[from];
        len++;
        from++;
    }
    dest[len] = '\0';
}

/* Copy src into dest without ever writing more than max characters. */
void copy_text(char *dest, int max, const char *src) {
    dest[0] = '\0';
    add_text(dest, max, src);
}

/* ---------------------------------------------------------------------- */
/* character tests                                                        */
/* ---------------------------------------------------------------------- */

int is_name_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

int is_blank(char c) {
    return c == ' ' || c == '\t';
}

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/* ---------------------------------------------------------------------- */
/* little string helpers                                                  */
/* ---------------------------------------------------------------------- */

void trim_right(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && is_space(s[len - 1])) {
        len--;
    }
    s[len] = '\0';
}

void trim_left(char *s) {
    int i = 0;
    int k;
    while (is_blank(s[i]))
        i++;
    if (i == 0)
        return;
    for (k = 0; s[i + k] != '\0'; k++)
        s[k] = s[i + k];
    s[k] = '\0';
}

int read_word(const char *s, int i, char *word, int max) {
    int k = 0;
    while (is_name_char(s[i]) && k < max - 1) {
        word[k] = s[i];
        k++;
        i++;
    }
    word[k] = '\0';
    return i;
}

/* ---------------------------------------------------------------------- */
/* paths                                                                  */
/* ---------------------------------------------------------------------- */

void dir_of(const char *path, char *dir, int max) {
    int i;
    int cut = -1; /* where the last '/' is, -1 means "no folder part" */

    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '/')
            cut = i;
    }
    if (cut < 0) {
        dir[0] = '\0';
        return;
    }
    add_slice(dir, max, path, 0, cut);
}

void join_path(const char *dir, const char *name, char *out, int max) {
    out[0] = '\0';
    if (name[0] == '/' || dir[0] == '\0') {
        add_text(out, max, name);
        return;
    }
    add_text(out, max, dir);
    add_char(out, max, '/');
    add_text(out, max, name);
}

/* ---------------------------------------------------------------------- */
/* quotes                                                                 */
/* ---------------------------------------------------------------------- */

int skip_quoted(const char *s, int i) {
    char quote = s[i];
    i++;
    while (s[i] != '\0' && s[i] != quote && s[i] != '\n') {
        if (s[i] == '\\' && s[i + 1] != '\0')
            i++; /* the backslash protects the next character */
        i++;
    }
    if (s[i] == quote)
        i++; /* step over the closing quote */
    return i;
}

int copy_quoted(const char *s, int i, char *out, int max) {
    int start = i;
    i = skip_quoted(s, i);
    add_slice(out, max, s, start, i);
    return i;
}

/* ---------------------------------------------------------------------- */
/* comments                                                               */
/* ---------------------------------------------------------------------- */

int comment_state_after(const char *line, int in_block) {
    int i = 0;

    while (line[i] != '\0') {
        if (in_block) {
            if (line[i] == '*' && line[i + 1] == '/') {
                in_block = 0;
                i += 2;
            } else {
                i++;
            }
        } else if (line[i] == '"' || line[i] == '\'') {
            i = skip_quoted(line, i);
        } else if (line[i] == '/' && line[i + 1] == '/') {
            return 0; /* the rest of the line is a comment */
        } else if (line[i] == '/' && line[i + 1] == '*') {
            in_block = 1;
            i += 2;
        } else {
            i++;
        }
    }
    return in_block;
}

/* ---------------------------------------------------------------------- */
/* preprocessor directives                                                */
/* ---------------------------------------------------------------------- */

int read_directive(const char *line, char *name, int max) {
    int i = 0;
    int k = 0;

    while (is_blank(line[i])) /* spaces before the '#' are allowed */
        i++;
    if (line[i] != '#')
        return -1;
    i++;
    while (is_blank(line[i])) /* "#  include" is allowed too */
        i++;

    while (is_name_char(line[i]) && k < max - 1) {
        name[k] = line[i];
        k++;
        i++;
    }
    name[k] = '\0';

    if (k == 0) /* a lone '#' is not useful to us */
        return -1;
    return i;
}
