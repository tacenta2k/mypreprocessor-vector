/* util.h -- small helpers that the other files share.
 *
 * Everything here works on ordinary char arrays.  There is no malloc and
 * no pointer-to-pointer: this project is meant to stay easy to read.
 */
#ifndef UTIL_H
#define UTIL_H

/* Biggest line, macro body and path we accept.
 * Keeping the limits in one place makes them easy to find and change. */
#define MAX_LINE 1024
#define MAX_PATH 512

/* --- building strings safely --------------------------------------------
 * These never write more than max-1 characters plus the final '\0'.
 * If the text does not fit it is cut short instead of corrupting memory.
 */
void add_char(char *dest, int max, char c);
void add_text(char *dest, int max, const char *text);
void add_slice(char *dest, int max, const char *src, int from, int to);
void copy_text(char *dest, int max, const char *src); /* a safe strcpy */

/* --- character tests ---------------------------------------------------- */
int is_name_char(char c); /* letters, digits and '_' : what a name is made of */
int is_blank(char c);     /* ' ' or '\t' only                                    */
int is_space(char c);     /* any whitespace, including '\n'                     */

/* --- little string helpers ---------------------------------------------- */
void trim_right(char *s); /* remove whitespace from the end of s   */
void trim_left(char *s);  /* remove spaces and tabs from the start */

/* Copy the name that starts at s[i] into word and return the position
 * just after it.  A name stops at the first character that cannot be
 * part of a name. */
int read_word(const char *s, int i, char *word, int max);

/* --- paths --------------------------------------------------------------- */
/* "src/prog.c" -> "src",  "prog.c" -> "",  "/a/b.c" -> "/a" */
void dir_of(const char *path, char *dir, int max);

/* Put dir and name together: dir="" gives "name", dir="/a" gives "/a/name".
 * A name that starts with '/' is already complete and is copied unchanged. */
void join_path(const char *dir, const char *name, char *out, int max);

/* --- quotes -------------------------------------------------------------
 * A "string" or a 'character' is copied exactly as written; a backslash
 * makes the next character part of it (so "a\"b" is one string).
 * copy_quoted() writes the text into out and returns the position just
 * after the closing quote.  skip_quoted() only finds that position. */
int copy_quoted(const char *s, int i, char *out, int max);
int skip_quoted(const char *s, int i);

/* --- comments -----------------------------------------------------------
 * Give the comment state at the end of one line: 0 means "normal code",
 * 1 means "still inside a block comment".  Used to ignore lines that only
 * look like directives because they are inside a comment. */
int comment_state_after(const char *line, int in_block);

/* --- preprocessor directives -------------------------------------------- */
/* Check whether a line is a directive such as "  #define X 1".
 * On success the directive word ("define", "include", ...) is copied into
 * name, and the position just after that word is returned.
 * Returns -1 when the line is not a directive. */
int read_directive(const char *line, char *name, int max);

#endif
