#!/bin/sh
# tests/run_tests.sh -- a small test suite for mypreprocessor.
#
#   From the project folder:   sh tests/run_tests.sh
#
# Each test writes a tiny .c file, runs the preprocessor and looks for a
# piece of text in the .i file.  Nothing needs to be installed; only a
# normal shell and the grep/printf commands are used.

set -u

PROGRAM="./mypreprocessor"
WORK="tests/tmp"
PASS=0
FAIL=0

if [ ! -x "$PROGRAM" ]; then
    echo "Please build the program first:  make"
    exit 1
fi

rm -rf "$WORK"
mkdir -p "$WORK"
cd "$WORK" || exit 1
cp "../../test_include.h" . 2>/dev/null || true
PROGRAM="../../mypreprocessor"

# check NAME FILE TEXT   -- TEXT must appear in FILE
check() {
    if grep -qF -- "$3" "$2" 2>/dev/null; then
        PASS=$((PASS + 1))
        printf '  ok    %s\n' "$1"
    else
        FAIL=$((FAIL + 1))
        printf '  FAIL  %s\n' "$1"
        printf '        expected to find: %s\n' "$3"
        printf '        in: %s\n' "$2"
    fi
}

# check_not NAME FILE TEXT  -- TEXT must NOT appear in FILE
check_not() {
    if grep -qF -- "$3" "$2" 2>/dev/null; then
        FAIL=$((FAIL + 1))
        printf '  FAIL  %s\n' "$1"
        printf '        did not expect: %s\n' "$3"
    else
        PASS=$((PASS + 1))
        printf '  ok    %s\n' "$1"
    fi
}

echo "comment removal"

cat > c1.c <<'EOF'
int a = 1; // this comment goes away
int b = 2; /* this one too */
int c = 3;
EOF
$PROGRAM c1.c >/dev/null
check "one line comment removed" c1.i "int a = 1; "
check "block comment removed" c1.i "int b = 2; "
check_not "comment text gone" c1.i "this comment goes away"

cat > c2.c <<'EOF'
char *url = "http://example.com"; // real comment
char brace = '{';
EOF
$PROGRAM c2.c >/dev/null
check "// inside a string is kept" c2.i '"http://example.com"'
check "quote inside a char is kept" c2.i "char brace = '{';"

cat > c3.c <<'EOF'
int a = 1; /* one
two
three */
int b = 2;
EOF
$PROGRAM c3.c >/dev/null
check "multi line comment removed" c3.i "int b = 2;"
check_not "multi line comment text gone" c3.i "two"

echo "file inclusion"

cat > inc.h <<'EOF'
int from_header = 1;
EOF
cat > i1.c <<'EOF'
#include "inc.h"
int main(void) { return from_header; }
EOF
$PROGRAM i1.c >/dev/null
check "header pasted in" i1.i "int from_header = 1;"
check_not "#include line kept" i1.i '#include "inc.h"'

cat > nested.h <<'EOF'
#include "inc.h"
int from_nested = 2;
EOF
cat > i2.c <<'EOF'
#include "nested.h"
EOF
$PROGRAM i2.c >/dev/null
check "header inside a header" i2.i "int from_header = 1;"
check "nested header itself" i2.i "int from_nested = 2;"

cat > i3.c <<'EOF'
#include "i3.c"
int main(void) { return 0; }
EOF
if timeout 5 $PROGRAM i3.c >/dev/null 2>&1; then
    PASS=$((PASS + 1))
    printf '  ok    a file including itself does not hang\n'
else
    if [ $? -eq 124 ]; then
        FAIL=$((FAIL + 1))
        printf '  FAIL  a file including itself hangs\n'
    else
        PASS=$((PASS + 1))
        printf '  ok    a file including itself stops\n'
    fi
fi

cat > i4.c <<'EOF'
/*
#include "inc.h"
*/
int main(void) { return 0; }
EOF
$PROGRAM i4.c >/dev/null
check_not "#include inside a comment is ignored" i4.i "from_header"

cat > i5.c <<'EOF'
  #include "inc.h"
  int main(void) { return from_header; }
EOF
$PROGRAM i5.c >/dev/null
check "indented #include works" i5.i "int from_header = 1;"

echo "macros"

cat > m1.c <<'EOF'
#define PI 3.14
float area = PI;
EOF
$PROGRAM m1.c >/dev/null
check "simple macro replaced" m1.i "float area = 3.14;"
check_not "#define line kept" m1.i "#define PI"

cat > m2.c <<'EOF'
#define ADD(a, b) ((a) + (b))
int sum = ADD(1, 2);
EOF
$PROGRAM m2.c >/dev/null
check "macro with arguments" m2.i "int sum = ((1) + (2));"

cat > m3.c <<'EOF'
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int big = MAX(1, MAX(2, 3));
EOF
$PROGRAM m3.c >/dev/null
check "macro used inside a macro call" m3.i "int big = ((1) > (((2) > (3) ? (2) : (3))) ? (1) : (((2) > (3) ? (2) : (3))));"

cat > m4.c <<'EOF'
#define MAX 100
int MAXIMUM = 1;
int value = MAX;
EOF
$PROGRAM m4.c >/dev/null
check "longer names are not touched" m4.i "int MAXIMUM = 1;"
check "whole names are replaced" m4.i "int value = 100;"

cat > m5.c <<'EOF'
#define X 1
#define X 2
int value = X;
EOF
$PROGRAM m5.c >/dev/null
check "the newest #define wins" m5.i "int value = 2;"

cat > m6.c <<'EOF'
#define NAME "world"
char *text = NAME;
char *other = "NAME";
EOF
$PROGRAM m6.c >/dev/null
check "macro replaced in code" m6.i 'char *text = "world";'
check "macro NOT replaced inside a string" m6.i 'char *other = "NAME";'

cat > m7.c <<'EOF'
#define SIZE 10
#define HALF (SIZE / 2)
int half = HALF;
EOF
$PROGRAM m7.c >/dev/null
check "macro inside a macro value" m7.i "int half = (10 / 2);"

cat > m8.c <<'EOF'
#define ADD(a, b) ((a) + (b))
int sum = ADD(1);
EOF
$PROGRAM m8.c >/dev/null
check "wrong argument count leaves the call alone" m8.i "int sum = ADD(1);"

cat > m9.c <<'EOF'
#define BIG(a, b) ((a) > (b) ? (a) : (b))
int value = BIG(f(1, 2), 3);
EOF
$PROGRAM m9.c >/dev/null
check "commas inside brackets are kept" m9.i "int value = ((f(1, 2)) > (3) ? (f(1, 2)) : (3));"

echo "command line"

$PROGRAM >/dev/null 2>&1
if [ $? -eq 1 ]; then
    PASS=$((PASS + 1)); printf '  ok    no arguments prints help and exits 1\n'
else
    FAIL=$((FAIL + 1)); printf '  FAIL  no arguments should exit 1\n'
fi

$PROGRAM nothere.c >/dev/null 2>&1
if [ $? -eq 2 ]; then
    PASS=$((PASS + 1)); printf '  ok    missing file exits 2\n'
else
    FAIL=$((FAIL + 1)); printf '  FAIL  missing file should exit 2\n'
fi

$PROGRAM file.txt >/dev/null 2>&1
if [ $? -eq 1 ]; then
    PASS=$((PASS + 1)); printf '  ok    wrong extension exits 1\n'
else
    FAIL=$((FAIL + 1)); printf '  FAIL  wrong extension should exit 1\n'
fi

echo "errors and edge cases"

# very long file name (there used to be a buffer overflow here)
LONG=$(printf 'a%.0s' $(seq 1 200)).c
printf 'int main(void){return 0;}\n' > "$LONG"
if timeout 5 $PROGRAM "$LONG" >/dev/null 2>&1; then
    PASS=$((PASS + 1)); printf '  ok    a 200 character file name works\n'
else
    FAIL=$((FAIL + 1)); printf '  FAIL  a 200 character file name crashes\n'
fi

# line longer than the internal buffer
{
    printf '#define N 5\nint main(void){ // '
    printf 'x%.0s' $(seq 1 1500)
    printf '\n return N; }\n'
} > long.c
if timeout 5 $PROGRAM long.c >/dev/null 2>&1; then
    PASS=$((PASS + 1)); printf '  ok    a very long line does not crash\n'
else
    FAIL=$((FAIL + 1)); printf '  FAIL  a very long line crashes\n'
fi
check "macro after a very long line still works" long.i "return 5;"

cd ../..
rm -rf "$WORK"

echo
echo "passed: $PASS   failed: $FAIL"
[ "$FAIL" -eq 0 ]
