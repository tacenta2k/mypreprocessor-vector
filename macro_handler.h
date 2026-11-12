#ifndef MACRO_HANDLER_H
#define MACRO_HANDLER_H

#include <stdio.h>

// copy in to out, store #define lines and replace macro names
// other # lines are copied unchanged
void process_macros(FILE *in, FILE *out);

#endif
