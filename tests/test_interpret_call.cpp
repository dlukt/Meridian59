#include "test_framework.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>

// Define UNIT_TEST_SENDMSG to skip heavy functions like InterpretAtMessage
#define UNIT_TEST_SENDMSG
// Define UNIT_TEST_INTERPRET_CALL to expose InterpretCall
#define UNIT_TEST_INTERPRET_CALL

// Mocks for dependencies needed by InterpretCall

// Include blakserv.h to get types
#include "../blakserv/blakserv.h"

// Globals referenced by InterpretCall
extern kod_statistics kod_stat;
extern char *bkod;

// Mocks

// Config
bool ConfigBool(int config_id) { (void)config_id; return false; }
int ConfigInt(int config_id) { (void)config_id; return 1000; }
UINT64 GetMilliCount(void) { return 0; }
time_t GetTime(void) { return 0; }

// Logging
void eprintf(const char *format, ...) { (void)format; }
void bprintf(const char *format, ...) { (void)format; }
void dprintf(const char *format, ...) { (void)format; }
void SendSessionAdminText(int session_id, const char *format, ...) { (void)session_id; (void)format; }
void FlushDefaultChannels(void) {}

// Strings
std::string TimeStr(time_t) { return ""; }
const char *GetNameByID(int id) { (void)id; return "MockName"; }
const char *GetTagName(val_type val) { (void)val.int_val; return "MockTag"; }
const char *GetDataName(val_type val) { (void)val.int_val; return "MockData"; }
std::string obj_to_string(int tag, INT64 data) { (void)tag; (void)data; return ""; }

// Objects
object_node *GetObjectByID(int id) {
    static object_node obj;
    static class_node cls;
    cls.class_name = (char*)"MockClass";
    obj.class_ptr = &cls;
    obj.class_id = 1;
    // Need properties for StoreValue/RetrieveValue if they access them
    static prop_type props[10];
    obj.p = props;
    cls.num_properties = 10;

    (void)id;
    return &obj;
}

class_node *GetClassByID(int id) {
    static class_node cls;
    (void)id;
    return &cls;
}

message_node *GetMessageByID(int class_id, int message_id, class_node **c_ret) {
    (void)class_id; (void)message_id; (void)c_ret;
    return nullptr;
}

// Needed by RetrieveValue
blak_int val32to64(unsigned int v) { return (blak_int)v; }

// Needed by SendTopLevelBlakodMessage
blak_int SendBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]) {
    (void)object_id; (void)message_id; (void)num_parms; (void)parms;
    return 0;
}

// Needed by BlakodDebugInfo
int GetSourceLine(class_node *c, char *bkod) {
    (void)c; (void)bkod;
    return 0;
}

// Include sendmsg.c to get InterpretCall
#include "../blakserv/sendmsg.c"

// Helper to write int to buffer (LSB first)
void write_int(std::vector<char>& buf, unsigned int val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

// Helper to write blak_int (which is 64 bit?)
// sendmsg.c: get_blakint() calls val32to64(get_int())
// So on bytecode side it is 32-bit int.
void write_blakint(std::vector<char>& buf, int val) {
    write_int(buf, (unsigned int)val);
}

// Mock C function
static bool g_c_func_called = false;
static blak_int Mock_C_Func(int object_id,local_var_type *local_vars,
                            int num_normal_parms,parm_node normal_parm_array[],
                            int num_name_parms,parm_node name_parm_array[]) {
    (void)object_id; (void)local_vars; (void)num_normal_parms; (void)normal_parm_array;
    (void)num_name_parms; (void)name_parm_array;
    g_c_func_called = true;
    return 0;
}

static int test_interpret_call_success(void)
{
    std::vector<char> bytecode;
    unsigned char func_id = 10;

    // Setup opcode: CALL
    opcode_type op;
    op.source1 = CALL_NO_ASSIGN;

    // Setup bytecode stream for InterpretCall
    // InterpretCall reads:
    // info (byte) -> func id
    // [if assign: assign_index (int)] -> skipped for CALL_NO_ASSIGN
    // num_normal_parms (byte)
    // normal params...
    // num_name_parms (byte)
    // name params...

    bytecode.push_back(func_id); // info

    bytecode.push_back(1); // num_normal_parms = 1
    // normal param: type (byte), value (blakint = 4 bytes)
    bytecode.push_back(TAG_INT);
    write_blakint(bytecode, 123);

    bytecode.push_back(0); // num_name_parms = 0

    // Setup bkod pointer
    bkod = bytecode.data();

    // Setup ccall_table
    ccall_table[func_id] = Mock_C_Func;
    g_c_func_called = false;

    // Setup locals
    local_var_type locals;
    locals.num_locals = 0;

    object_node *o = GetObjectByID(1);

    // Call
    bool result = InterpretCall(&o, 1, &locals, op);

    ASSERT_TRUE(result);
    ASSERT_TRUE(g_c_func_called);

    return 0;
}

static int test_interpret_call_fail_max_c_parms(void)
{
    std::vector<char> bytecode;
    unsigned char func_id = 11;

    // Setup opcode
    opcode_type op;
    op.source1 = CALL_NO_ASSIGN;

    bytecode.push_back(func_id); // info

    // num_normal_parms > MAX_C_PARMS (40)
    bytecode.push_back(MAX_C_PARMS + 1);

    // We don't need to put the rest because it should fail immediately

    bkod = bytecode.data();
    ccall_table[func_id] = Mock_C_Func;
    g_c_func_called = false;

    local_var_type locals;
    object_node *o = GetObjectByID(1);

    bool result = InterpretCall(&o, 1, &locals, op);

    ASSERT_TRUE(!result);
    ASSERT_TRUE(!g_c_func_called);

    return 0;
}

static int test_interpret_call_fail_max_name_parms(void)
{
    std::vector<char> bytecode;
    unsigned char func_id = 12;

    // Setup opcode
    opcode_type op;
    op.source1 = CALL_NO_ASSIGN;

    bytecode.push_back(func_id); // info

    bytecode.push_back(0); // num_normal_parms = 0

    // num_name_parms > MAX_NAME_PARMS (45)
    bytecode.push_back(MAX_NAME_PARMS + 1);

    bkod = bytecode.data();
    ccall_table[func_id] = Mock_C_Func;
    g_c_func_called = false;

    local_var_type locals;
    object_node *o = GetObjectByID(1);

    bool result = InterpretCall(&o, 1, &locals, op);

    ASSERT_TRUE(!result);
    ASSERT_TRUE(!g_c_func_called);

    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_interpret_call_success", test_interpret_call_success, &tests_run);
    failures += run_test("test_interpret_call_fail_max_c_parms", test_interpret_call_fail_max_c_parms, &tests_run);
    failures += run_test("test_interpret_call_fail_max_name_parms", test_interpret_call_fail_max_name_parms, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
