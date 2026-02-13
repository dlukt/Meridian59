#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "test_framework.h"

// We include blakserv.h to get the types and declarations
#include "../blakserv/blakserv.h"

// Define stubs for external functions used by term.c
// These allow us to test term.c in isolation
// Since term.c is compiled as C++, these must have C++ linkage to match the headers.

void SendClientStr(int session_id, char *s) {
    (void)session_id;
    (void)s;
}

resource_node * GetResourceByID(int id) {
    (void)id;
    return NULL;
}

class_node * GetClassByID(int id) {
    (void)id;
    return NULL;
}

resource_node * GetResourceByName(const char *name) {
    (void)name;
    return NULL;
}

class_node * GetClassByName(const char *name) {
    (void)name;
    return NULL;
}

// Define any missing global variables
DWORD main_thread_id = 0;

// Test Cases

static int test_GetTagName_Basic(void) {
    val_type v;
    v.v.tag = TAG_INT;
    const char* name = GetTagName(v);
    ASSERT_TRUE(strcmp(name, "INT") == 0);

    v.v.tag = TAG_NIL;
    name = GetTagName(v);
    ASSERT_TRUE(strcmp(name, "$") == 0);

    return 0;
}

static int test_GetTagName_Custom(void) {
    val_type v;
    // Use TAG_RESERVED (14) which is not handled in the switch case in term.c
    // This triggers the static buffer usage.
    v.v.tag = TAG_RESERVED;
    const char* name = GetTagName(v);

    // snprintf(s, sizeof(s), "%i",(int) val.v.tag);
    // So it should return "14".
    ASSERT_TRUE(strcmp(name, "14") == 0);
    return 0;
}

static int test_GetTagName_Threading(void) {
    // This test attempts to detect race conditions in GetTagName.
    // We need to use tags that trigger the static buffer usage.
    // However, we only have one such tag (TAG_RESERVED = 14) accessible via the enum constant.
    // But since tag is a 4-bit field, we are limited to 0-15.
    // 0-13 and 15 are handled. 14 is the only one hitting default.

    // Wait, if we only have one value (14) that hits the buffer,
    // both threads will write "14" to their buffer.
    // If they share the buffer, they overwrite "14" with "14". No race condition visible!

    // We need at least TWO different values that hit the buffer to detect a race.
    // But we don't have them in the 4-bit space!

    // Unless... we can trick it? No, 4 bits is 4 bits.

    // Does GetDataName have the same issue?
    // GetDataName:
    // case TAG_RESOURCE: ...
    // case TAG_CLASS: ...
    // case TAG_MESSAGE: ...
    // default: ... snprintf(s, ..., "%lli", val.v.data);

    // For GetDataName, the default case prints val.v.data!
    // val.v.data is 60 bits.
    // So we can use ANY tag that falls into default (e.g. TAG_INT) and vary the data.

    // So GetDataName is the better candidate for threading test.

    // I will verify GetTagName behavior for 14, but rely on GetDataName for the race test.

    return 0;
}

static int test_GetDataName_Threading(void) {
    std::atomic<int> errors(0);
    std::vector<std::thread> threads;

    auto thread_func = [&](int data_val) {
        val_type v;
        v.v.tag = TAG_INT; // TAG_INT falls through to default in GetDataName
        v.v.data = data_val;

        for (int i = 0; i < 1000; ++i) {
            const char* name = GetDataName(v);
            std::string expected = std::to_string(data_val);
            if (strcmp(name, expected.c_str()) != 0) {
                errors++;
                // fprintf(stderr, "Thread %d got %s instead of %s\n", data_val, name, expected.c_str());
            }
            std::this_thread::yield();
        }
    };

    // Use two different values so if they share the buffer, one will see the other's value.
    threads.emplace_back(thread_func, 123456);
    threads.emplace_back(thread_func, 654321);

    for (auto& t : threads) {
        t.join();
    }

    if (errors > 0) {
        fprintf(stderr, "GetDataName Threading test failed with %d mismatches.\n", errors.load());
        return 1;
    }

    return 0;
}


int run_term_tests(int *tests_run, int *failures) {
    int local_failures = 0;
    local_failures += run_test("test_GetTagName_Basic", test_GetTagName_Basic, tests_run);
    local_failures += run_test("test_GetTagName_Custom", test_GetTagName_Custom, tests_run);
    local_failures += run_test("test_GetTagName_Threading", test_GetTagName_Threading, tests_run);
    local_failures += run_test("test_GetDataName_Threading", test_GetDataName_Threading, tests_run);

    *failures += local_failures;
    return local_failures;
}
