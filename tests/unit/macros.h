#ifndef TMACROS_H
#define TMACROS_H

#include <stdio.h>

#define TEST_LOG(message) \
  printf("[TEST] %s\n", message)

#define TEST_PASS() \
    printf("[PASS] %s\n", __func__); \
    return 0

#define TEST_FAIL(message) \
  do { \
    fprintf(stderr, "[FAIL] %s: %s (%s:%d)\n", \
            __func__, message, __FILE__, __LINE__); \
    return 1; \
  } while (0)

#define TEST_ASSERT(condition) \
  do { \
    if (!(condition)) { \
      TEST_FAIL(#condition); \
    } \
  } while (0)

#define TEST_ASSERT_EQ(expected, actual) \
  do { \
    if ((expected) != (actual)) { \
      TEST_FAIL(#expected " != " #actual); \
    } \
  } while (0)

#define TEST_ASSERT_NE(expected, actual) \
  do { \
    if ((expected) == (actual)) { \
      TEST_FAIL(#expected " == " #actual); \
    } \
  } while (0)

#define TEST_ASSERT_NULL(value) \
  TEST_ASSERT((value) == NULL)

#define TEST_ASSERT_NOT_NULL(value) \
  TEST_ASSERT((value) != NULL)

#endif
