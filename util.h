// shared helpers
#ifndef UTIL_H
#define UTIL_H

#define MAX_LINE 1024
#define MAX_PATH 512

// safe string building, never write past max
void add_char(char *dest, int max, char c);
void add_text(char *dest, int max, const char *text);
void add_slice(char *dest, int max, const char *src, int from, int to);
void copy_text(char *dest, int max, const char *src);

// char checks
int is_name_char(char c); // a-z A-Z 0-9 _
int is_blank(char c);     // space or tab
int is_space(char c);

void trim_right(char *s);
void trim_left(char *s);
int read_word(const char *s, int i, char *word, int max);

// paths
void dir_of(const char *path, char *dir, int max);
void join_path(const char *dir, const char *name, char *out, int max);

// quotes, i points at " or '
int copy_quoted(const char *s, int i, char *out, int max);
int skip_quoted(const char *s, int i);

// 1 if we are still inside a block comment after this line
int comment_state_after(const char *line, int in_block);

// line like "  #define X 1" -> fills name, returns pos after name, -1 if no #
int read_directive(const char *line, char *name, int max);

#endif
