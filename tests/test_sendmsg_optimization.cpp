#include "test_framework.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>

// Define UNIT_TEST_SENDMSG to skip heavy functions in sendmsg.c
#define UNIT_TEST_SENDMSG

// Rename globals to avoid linker collisions
#define kod_stat test_kod_stat
#define message_depth test_message_depth
#define stack test_stack
#define bkod test_bkod
#define num_interpreted test_num_interpreted
#define trace_session_id test_trace_session_id
#define post_q test_post_q
#define ccall_table test_ccall_table
#define done test_done

// Include blakserv.h to get types
#include "../blakserv/blakserv.h"

// Undefine macros before including system headers or if they conflict
// But we need them active when including sendmsg.c

// Mock dependencies
static bool g_mock_debug_initlocals = false;
static int g_mock_milli_count = 1000;

bool ConfigBool(int config_id) {
    if (config_id == DEBUG_INITLOCALS) {
        return g_mock_debug_initlocals;
    }
    // Default for DEBUG_UNINITIALIZED
    return false;
}

int ConfigInt(int config_id) {
    (void)config_id;
    return 1000; // BLAKOD_MAX_STATEMENTS
}

UINT64 GetMilliCount(void) {
    return g_mock_milli_count;
}

time_t GetTime(void) {
    return (time_t)(g_mock_milli_count / 1000);
}

// Stubs for logging and other functions used by SendTopLevelBlakodMessage
void eprintf(const char *format, ...) { (void)format; }
void bprintf(const char *format, ...) { (void)format; }
void dprintf(const char *format, ...) { (void)format; }
void SendSessionAdminText(int session_id, const char *format, ...) { (void)session_id; (void)format; }
void FlushDefaultChannels(void) {}

const char *GetNameByID(int id) { (void)id; return "MockName"; }
const char *GetTagName(val_type val) { (void)val.int_val; return "MockTag"; }
const char *GetDataName(val_type val) { (void)val.int_val; return "MockData"; }

object_node *GetObjectByID(int id) {
    static object_node obj;
    static class_node cls;
    cls.class_name = (char*)"MockClass";
    obj.class_ptr = &cls; // For dprintf access
    obj.class_id = 1;
    (void)id;
    return &obj;
}

class_node *GetClassByID(int id) {
    static class_node cls;
    cls.class_name = (char*)"MockClass";
    (void)id;
    return &cls;
}

message_node *GetMessageByID(int class_id, int message_id, class_node **c_ret) {
    (void)class_id; (void)message_id; (void)c_ret;
    return nullptr;
}

// Stub for GetSourceLine which is called by BlakodDebugInfo and BlakodStackInfo
int GetSourceLine(class_node *c, char *bkod) {
    (void)c; (void)bkod;
    return 0;
}

// Stub for SendBlakodMessage which is called by SendTopLevelBlakodMessage
blak_int SendBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]) {
    // We don't need to implement logic, just return something
    (void)object_id; (void)message_id; (void)num_parms; (void)parms;
    return 0;
}

// Include sendmsg.c to test InitProfiling and SendTopLevelBlakodMessage
#include "../blakserv/sendmsg.c"

// Test functions

static int test_init_profiling_sets_debug_initlocals(void)
{
    // Reset state
    test_done = 0;
    g_mock_debug_initlocals = true;

    InitProfiling();

    ASSERT_TRUE(test_kod_stat.debug_initlocals);

    // Test false case
    test_done = 0;
    g_mock_debug_initlocals = false;

    InitProfiling();

    ASSERT_TRUE(!test_kod_stat.debug_initlocals);

    return 0;
}

static int test_send_top_level_refresh(void)
{
    // Reset state
    test_done = 0;
    InitProfiling(); // Initialize first

    // Set config to true
    g_mock_debug_initlocals = true;

    // Call top level message
    parm_node parms[1];
    SendTopLevelBlakodMessage(1, 1, 0, parms);

    ASSERT_TRUE(test_kod_stat.debug_initlocals);

    // Set config to false
    g_mock_debug_initlocals = false;
    SendTopLevelBlakodMessage(1, 1, 0, parms);

    ASSERT_TRUE(!test_kod_stat.debug_initlocals);

    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_init_profiling_sets_debug_initlocals", test_init_profiling_sets_debug_initlocals, &tests_run);
    failures += run_test("test_send_top_level_refresh", test_send_top_level_refresh, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
