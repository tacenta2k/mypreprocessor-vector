#ifndef FILE_INCLUSION_H
#define FILE_INCLUSION_H

#include <stdio.h>

// copy in to out, replace #include lines with the file contents
// path is used for the folder of "..." includes
// returns 1 if a header was missing
int include_files(FILE *in, FILE *out, const char *path);

#endif
