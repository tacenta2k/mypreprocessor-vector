/* file_inclusion.h -- stage 2 of the preprocessor: paste in #include files. */
#ifndef FILE_INCLUSION_H
#define FILE_INCLUSION_H

#include <stdio.h>

/* Copy "in" to "out", replacing every #include line with the contents of
 * the file it names:
 *
 *   #include <stdio.h>   searched in /usr/include and /usr/local/include
 *   #include "my.h"      searched next to the file that has the #include
 *
 * "path" is the name of the file being read, e.g. "test.c" or "src/prog.c".
 * Only its folder is used, and only for the "..." form, so that a program
 * in another folder still finds its own headers.
 *
 * Returns 0 when everything was fine, 1 when at least one header could not
 * be found (its #include line is then copied unchanged).
 */
int include_files(FILE *in, FILE *out, const char *path);

#endif
