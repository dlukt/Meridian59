#include "test_framework.h"
#include <cstring>
#include <cstdio>
#include "../blakserv/osd_linux.h"

// Define a test case for FormatFatalError
int test_format_fatal_error() {
    char buf[1024];
    const char* filename = "test_file.c";
    int line = 123;
    const char* message = "Test fatal error";

    // Call the function we are testing
    FormatFatalError(buf, sizeof(buf), filename, line, message);

    // Check if the output string matches the expected format
    char expected[1024];
    snprintf(expected, sizeof(expected), "Fatal Error File %s line %i\r\n\r\n%s\n", filename, line, message);

    ASSERT_TRUE(strcmp(buf, expected) == 0);
    return 0;
}

// Another test case to verify buffer overflow protection
int test_format_fatal_error_buffer_limit() {
    char buf[20];
    const char* filename = "long_filename.c";
    int line = 999;
    const char* message = "Very long error message that should be truncated";

    FormatFatalError(buf, sizeof(buf), filename, line, message);

    // Ensure the buffer is null-terminated and not completely empty
    ASSERT_TRUE(strlen(buf) > 0);
    ASSERT_TRUE(strlen(buf) < sizeof(buf));
    return 0;
}
