/* comment_removal.h -- stage 1 of the preprocessor: throw comments away. */
#ifndef COMMENT_REMOVAL_H
#define COMMENT_REMOVAL_H

#include <stdio.h>

/* Copy everything from "in" to "out", but leave out comments.
 *
 *   // a one line comment        -> removed, the newline is kept
 *   a block comment             -> removed, a single space is put in its place
 *
 * Text inside "..." and '...' is never touched, so printf("http://x") keeps
 * its slashes.  Newlines are always kept so that error messages from the
 * compiler still point at the right line.
 */
void remove_comments(FILE *in, FILE *out);

#endif
