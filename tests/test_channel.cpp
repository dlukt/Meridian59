#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "test_framework.h"
#include "test_channel.h"
#include "../blakserv/blakserv.h"

// Capture buffer
static std::string last_channel_msg;
static int last_channel_id = -1;

// Stubs
void WriteChannelBuffer(int channel_id, char *s) {
    last_channel_id = channel_id;
    if (s) {
        last_channel_msg = s;
    } else {
        last_channel_msg.clear();
    }
}

bool ConfigBool(int config_id) {
    (void)config_id;
    return false;
}

char * ConfigStr(int config_id) {
    (void)config_id;
    return (char*)"";
}

void StartupPrintf(const char *fmt, ...) {
    (void)fmt;
}

std::string BlakodDebugInfo(void) {
    return "DebugInfo";
}

// Tests

static int test_dprintf_overflow_with_newlines(void) {
    // dprintf buffer is 2000.
    // We want to fill it using TermConvertBuffer expansion.
    // vsnprintf limits input to 1900 chars (2000 - MAX_TIME_SIZE(100)).
    // If we provide 1000 newlines, vsnprintf writes 1000 chars.
    // TermConvertBuffer expands them to 2000 chars -> Truncates at 1999.
    // Then AppendNewlineSafe should prevent overflow (which would occur with strcat).

    char input[1200];
    memset(input, '\n', sizeof(input));
    input[1199] = '\0'; // 1199 newlines.

    dprintf("%s", input);

    ASSERT_TRUE(last_channel_id == CHANNEL_D);

    size_t len = last_channel_msg.length();
    // Buffer size 2000.
    ASSERT_TRUE(len < 2000);

    // Should end with \r\n
    ASSERT_TRUE(len >= 2);
    ASSERT_TRUE(last_channel_msg[len-2] == '\r');
    ASSERT_TRUE(last_channel_msg[len-1] == '\n');

    // Should be very long (close to 2000)
    ASSERT_TRUE(len > 1900);

    return 0;
}

static int test_lprintf_basic(void) {
    lprintf("Hello");

    ASSERT_TRUE(last_channel_id == CHANNEL_L);
    size_t len = last_channel_msg.length();
    ASSERT_TRUE(len < 1000);
    ASSERT_TRUE(len >= 2);
    ASSERT_TRUE(last_channel_msg[len-2] == '\r');
    ASSERT_TRUE(last_channel_msg[len-1] == '\n');

    return 0;
}

int run_channel_tests(int *tests_run, int *failures) {
    int local_failures = 0;
    local_failures += run_test("test_dprintf_overflow_with_newlines", test_dprintf_overflow_with_newlines, tests_run);
    local_failures += run_test("test_lprintf_basic", test_lprintf_basic, tests_run);
    *failures += local_failures;
    return local_failures;
}
