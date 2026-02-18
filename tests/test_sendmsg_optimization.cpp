#include "test_framework.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <cstring>

// Ensure UNIT_TEST_SENDMSG is NOT defined to test the real interpreter logic
// #define UNIT_TEST_SENDMSG

// Rename globals to avoid linker collisions if we link against other objects
// But since we include .c file directly, these become our definitions
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

// channel.h defines fmt as macro calling obj_to_string
// We implement obj_to_string to satisfy the linker/compiler
std::string obj_to_string(int tag, INT64 data) {
    (void)tag; (void)data;
    return "MockFmt";
}

// C_Invalid stub needed for InitBkodInterpret
blak_int C_Invalid(int object_id,local_var_type *local_vars,
                               int num_normal_parms,parm_node normal_parm_array[],
                               int num_name_parms,parm_node name_parm_array[]) {
    (void)object_id; (void)local_vars; (void)num_normal_parms;
    (void)normal_parm_array; (void)num_name_parms; (void)name_parm_array;
    return 0;
}

// Stub C functions to avoid linking all of ccode.c
#define STUB_C_FUNC(name) blak_int name(int o, local_var_type *l, int n1, parm_node p1[], int n2, parm_node p2[]) { (void)o; (void)l; (void)n1; (void)p1; (void)n2; (void)p2; return 0; }

STUB_C_FUNC(C_CreateObject)
STUB_C_FUNC(C_IsClass)
STUB_C_FUNC(C_GetClass)
STUB_C_FUNC(C_SendMessage)
STUB_C_FUNC(C_PostMessage)
STUB_C_FUNC(C_AddPacket)
STUB_C_FUNC(C_SendPacket)
STUB_C_FUNC(C_SendCopyPacket)
STUB_C_FUNC(C_ClearPacket)
STUB_C_FUNC(C_Debug)
STUB_C_FUNC(C_GetInactiveTime)
STUB_C_FUNC(C_DumpStack)
STUB_C_FUNC(C_StringEqual)
STUB_C_FUNC(C_StringContain)
STUB_C_FUNC(C_SetResource)
STUB_C_FUNC(C_ParseString)
STUB_C_FUNC(C_SetString)
STUB_C_FUNC(C_AppendTempString)
STUB_C_FUNC(C_ClearTempString)
STUB_C_FUNC(C_GetTempString)
STUB_C_FUNC(C_CreateString)
STUB_C_FUNC(C_StringSubstitute)
STUB_C_FUNC(C_BuildString)
STUB_C_FUNC(C_StringLength)
STUB_C_FUNC(C_StringConsistsOf)
STUB_C_FUNC(C_CreateTimer)
STUB_C_FUNC(C_DeleteTimer)
STUB_C_FUNC(C_GetTimeRemaining)
STUB_C_FUNC(C_LoadRoom)
STUB_C_FUNC(C_RoomData)
STUB_C_FUNC(C_CanMoveInRoom)
STUB_C_FUNC(C_CanMoveInRoomFine)
STUB_C_FUNC(C_IsPointInSector)
STUB_C_FUNC(C_Cons)
STUB_C_FUNC(C_First)
STUB_C_FUNC(C_Rest)
STUB_C_FUNC(C_Length)
STUB_C_FUNC(C_Nth)
STUB_C_FUNC(C_List)
STUB_C_FUNC(C_IsList)
STUB_C_FUNC(C_SetFirst)
STUB_C_FUNC(C_SetNth)
STUB_C_FUNC(C_DelListElem)
STUB_C_FUNC(C_FindListElem)
STUB_C_FUNC(C_MoveListElem)
STUB_C_FUNC(C_GetTime)
STUB_C_FUNC(C_CreateTable)
STUB_C_FUNC(C_AddTableEntry)
STUB_C_FUNC(C_GetTableEntry)
STUB_C_FUNC(C_DeleteTableEntry)
STUB_C_FUNC(C_DeleteTable)
STUB_C_FUNC(C_IsObject)
STUB_C_FUNC(C_RecycleUser)
STUB_C_FUNC(C_Random)
STUB_C_FUNC(C_Abs)
STUB_C_FUNC(C_Bound)
STUB_C_FUNC(C_Sqrt)
STUB_C_FUNC(C_MinigameNumberToString)
STUB_C_FUNC(C_MinigameStringToNumber)
STUB_C_FUNC(C_SendWebhook)

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

int GetSourceLine(class_node *c, char *bkod) { (void)c; (void)bkod; return 0; }

void ForEachObject(void (*func)(object_node *)) {
    // No-op for now
    (void)func;
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

// We can't easily test SendTopLevelBlakodMessage flow because it calls SendBlakodMessage
// which we mock or implement. But since we included sendmsg.c, SendBlakodMessage is REAL.
// It calls GetObjectByID, GetClassByID, GetMessageByID.
// If we want to test InterpretAtMessage directly, we can do that.

static int test_interpret_return_constant(void)
{
    // Setup opcode buffer for: RETURN CONSTANT 42
    // Opcode: RETURN(4) | DEST(0) | SOURCE1(CONSTANT=2) | SOURCE2(0)
    // 4 << 5 = 128 (0x80)
    // 0 << 4 = 0
    // 2 << 2 = 8   (0x08)
    // 0 << 0 = 0
    // Total: 0x88

    unsigned char bytecode[32];
    memset(bytecode, 0, sizeof(bytecode));

    bytecode[0] = 0; // num_locals
    bytecode[1] = 0; // num_parms
    // Loop of parms skipped (0)

    // Instruction 1: RETURN CONSTANT 42
    bytecode[2] = 0x88;

    // Constant 42 (0x2A) as 4-byte int (Little Endian)
    // Assuming host is little endian (Linux x86/x64)
    *((unsigned int*)&bytecode[3]) = 42;

    // Set global bkod pointer
    test_bkod = (char*)bytecode;
    test_num_interpreted = 0;
    test_message_depth = 0; // Reset depth

    // Setup dummy args
    val_type ret_val;
    ret_val.int_val = 0;

    // Call interpreter
    int result = InterpretAtMessage(1, NULL, NULL, 0, NULL, &ret_val);

    // Verify result
    // Should return RETURN_NO_PROPAGATE (2)
    ASSERT_EQ_UINT(result, 2);

    // Verify return value
    ASSERT_EQ_UINT(ret_val.int_val, 42);

    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    // Initialize mock table for C calls
    InitBkodInterpret();

    failures += run_test("test_init_profiling_sets_debug_initlocals", test_init_profiling_sets_debug_initlocals, &tests_run);
    failures += run_test("test_interpret_return_constant", test_interpret_return_constant, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
