#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

typedef int (*test_fn)(void);

static inline int run_test(const char *name, test_fn fn, int *tests_run)
{
    int result;

    (*tests_run)++;
    result = fn();
    if (result != 0)
    {
        fprintf(stderr, "FAIL: %s\n", name);
        return 1;
    }

    printf("PASS: %s\n", name);
    return 0;
}

#define ASSERT_TRUE(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            fprintf(stderr, "Assertion failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define ASSERT_EQ_UINT(expected, actual) \
    do \
    { \
        if ((expected) != (actual)) \
        { \
            fprintf(stderr, "Expected 0x%08x but got 0x%08x (%s:%d)\n", \
                (unsigned int)(expected), (unsigned int)(actual), __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define ASSERT_EQ_MEM(expected, actual, size) \
    do \
    { \
        if (memcmp((expected), (actual), (size)) != 0) \
        { \
            fprintf(stderr, "Memory mismatch (%s:%d)\n", __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#endif /* TEST_FRAMEWORK_H */
