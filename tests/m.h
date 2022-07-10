/* m.h
A simple assertion macro.

*   `test_case` macro is used to define a test case.
It should at least hold one of the `assert` macros.
Note that `assert` macros may not produce the desired output if not used in a
test case.

*   `assert` macro is used to check the truth evaluation of an expression.
If the expression evaluates to false, the program will be terminated with an
error
*   `assert_eq` macro is used to check the equality of two values.
This macro, unlike `assert`, tries to cast the values to the same type before
evaluation. The type of the first argument is arbitrarily chosen.
*   `assert_neq`, `assert_lt`, `assert_gt`, `assert_leq`, `assert_geq` macros
are used to check the inequality of two values.
*   only use `assert_info` when the evaluated expression does not expand to it's
original form (for example, `assert_info(_a == _b, a == b)` if `a` and `b` are
casted to the same type in `_a` and `_b`).
*   `cast_to_same_type` macro is used to cast two values to the same type.
As specified before, the type of the first argument is arbitrarily chosen.
This macro thus creates two temporary variables, `_a` and `_b` that will be
used to perform checks.

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

extern unsigned long _no_asserts;

#define assert_info(expr, ...)                                   \
  do {                                                           \
    _no_asserts++;                                               \
    if (!(expr)) {                                               \
      fprintf(stderr, "\033[0;31m");                             \
      fprintf(stderr, "\nassertion failed: %s\n", #__VA_ARGS__); \
      fprintf(stderr, "\033[0m");                                \
      abort();                                                   \
    }                                                            \
  } while (0);

#ifndef assert
#define assert(expr) assert_info(expr, expr);
#endif

#define cast_to_same_type(a, b) \
  __typeof__(a) _a = (a);       \
  __typeof__(a) _b = (b);

#define assert_eq(a, b)            \
  do {                             \
    cast_to_same_type(a, b);       \
    assert_info(_a == _b, a == b); \
  } while (0);
#define assert_neq(a, b)           \
  do {                             \
    cast_to_same_type(a, b);       \
    assert_info(_a != _b, a != b); \
  } while (0);
#define assert_lt(a, b)          \
  do {                           \
    cast_to_same_type(a, b);     \
    assert_info(_a < _b, a < b); \
  } while (0);
#define assert_gt(a, b)          \
  do {                           \
    cast_to_same_type(a, b);     \
    assert_info(_a > _b, a > b); \
  } while (0);
#define assert_leq(a, b)           \
  do {                             \
    cast_to_same_type(a, b);       \
    assert_info(_a <= _b, a <= b); \
  } while (0);
#define assert_geq(a, b)           \
  do {                             \
    cast_to_same_type(a, b);       \
    assert_info(_a >= _b, a >= b); \
  } while (0);

#define test_case(name)                                        \
  do {                                                         \
    _no_asserts = 0;                                           \
    fprintf(stderr, "running %-20s:: %-20s", __FILE__, #name); \
    name();                                                    \
    if (_no_asserts > 0) {                                     \
      fprintf(stderr, "\033[0;32m");                           \
      fprintf(stderr, "ok (%lu)\n", _no_asserts);              \
      fprintf(stderr, "\033[0m");                              \
    } else {                                                   \
      fprintf(stderr, "\033[0;33m");                           \
      fprintf(stderr, "fake (0)\n");                           \
      fprintf(stderr, "\033[0m");                              \
    }                                                          \
  } while (0);
