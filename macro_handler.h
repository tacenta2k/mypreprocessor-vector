/* macro_handler.h -- stage 3 of the preprocessor: #define and expansion. */
#ifndef MACRO_HANDLER_H
#define MACRO_HANDLER_H

#include <stdio.h>

/* Copy "in" to "out", handling the #define lines.
 *
 *   #define PI 3.14              simple macro
 *   #define ADD(a, b) ((a)+(b))  macro with parameters
 *
 * A #define line is stored and left out of the output; every later use of
 * the name is replaced by its value.  A value that itself contains a macro
 * is expanded again, so
 *
 *   #define SIZE 10
 *   #define HALF (SIZE / 2)
 *
 * turns HALF into (10 / 2).  Names inside "strings" and 'characters' are
 * never replaced.  All other # lines (#include, #ifdef, ...) are copied
 * unchanged because they belong to the compiler's job, not ours.
 */
void process_macros(FILE *in, FILE *out);

#endif
