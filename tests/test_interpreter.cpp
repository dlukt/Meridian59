#include "test_framework.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <stdarg.h>

// Define UNIT_TEST_SENDMSG to exclude SendClassMessage/SendBlakodMessage blocks
#define UNIT_TEST_SENDMSG
// Define UNIT_TEST_INTERPRETER to include InterpretCall block
#define UNIT_TEST_INTERPRETER

// Rename globals to avoid linker collisions with other tests if linked together (though separate exe)
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

// Mocks for Config
bool ConfigBool(int config_id) { (void)config_id; return false; }
int ConfigInt(int config_id) { (void)config_id; return 1000; } // BLAKOD_MAX_STATEMENTS

// Mocks for Time
UINT64 GetMilliCount(void) { return 1000; }
time_t GetTime(void) { return 1; }

// Mocks for Logging
void eprintf(const char *format, ...) { (void)format; }
void bprintf(const char *format, ...) { (void)format; }
void dprintf(const char *format, ...) { (void)format; }
void SendSessionAdminText(int session_id, const char *format, ...) { (void)session_id; (void)format; }

static bool g_flush_called = false;
void FlushDefaultChannels(void) { g_flush_called = true; }

// Mocks for Names/Tags
const char *GetNameByID(int id) { (void)id; return "MockName"; }
const char *GetTagName(val_type val) { (void)val.int_val; return "MockTag"; }
const char *GetDataName(val_type val) { (void)val.int_val; return "MockData"; }

// Mocks for Objects/Classes
object_node *GetObjectByID(int id) {
    static object_node obj;
    static class_node cls;
    cls.class_name = (char*)"MockClass";
    obj.class_ptr = &cls;
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

// Mock for SendBlakodMessage (called by SendTopLevelBlakodMessage if it were compiled, but excluded)
// InterpretCall doesn't call this.
blak_int SendBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]) {
    (void)object_id; (void)message_id; (void)num_parms; (void)parms;
    return 0;
}

// Mock C function for InterpretCall to call
blak_int MockCFunc(int object_id, local_var_type *local_vars,
                  int num_normal_parms, parm_node normal_parm_array[],
                  int num_name_parms, parm_node name_parm_array[]) {
    (void)object_id; (void)local_vars; (void)num_normal_parms;
    (void)normal_parm_array; (void)num_name_parms; (void)name_parm_array;
    return 42; // arbitrary return value
}

// Mock for obj_to_string (helper for fmt macro used in InterpretUnaryAssign etc)
std::string obj_to_string(int tag, INT64 data) {
    (void)tag; (void)data;
    return "MockVal";
}

// Mock for GetSourceLine (used in BlakodDebugInfo)
int GetSourceLine(class_node *c, char *bkod_ptr) {
    (void)c; (void)bkod_ptr;
    return 1;
}

// Include source file
#include "../blakserv/sendmsg.c"

// Helper to construct bkod
void append_byte(std::vector<char>& buf, unsigned char b) {
    buf.push_back((char)b);
}

void append_int(std::vector<char>& buf, unsigned int i) {
    buf.push_back((char)(i & 0xFF));
    buf.push_back((char)((i >> 8) & 0xFF));
    buf.push_back((char)((i >> 16) & 0xFF));
    buf.push_back((char)((i >> 24) & 0xFF));
}

void append_blakint(std::vector<char>& buf, blak_int bi) {
    // Blakserv stores blak_int as 64-bit at runtime but 32-bit in bof/bytecode usually?
    // sendmsg.c uses `get_blakint` which calls `val32to64(get_int())`.
    // So `get_int` reads 4 bytes.
    // Wait, `get_int` reads 4 bytes. `get_blakint` reads 4 bytes and converts.
    // So we append 4 bytes.
    append_int(buf, (unsigned int)bi);
}

// Tests

static int test_InterpretCall_Valid(void) {
    // Setup
    test_ccall_table[1] = MockCFunc; // Use index 1
    g_flush_called = false;

    // Construct valid CALL opcode
    // Opcode struct: source2:2, source1:2, dest:1, command:3
    // InterpretCall uses opcode.source1 for assignment type.
    // Let's say CALL_NO_ASSIGN (0).
    // Command is CALL (which is 4).
    // So command=4, dest=0, source1=0, source2=0.
    // Byte: (0) | (0<<2) | (0<<4) | (4<<5) = 128 (0x80) ?
    // Wait, bit layout:
    // source2 (bits 0-1)
    // source1 (bits 2-3)
    // dest (bit 4)
    // command (bits 5-7)
    // 4 << 5 = 128.

    // Actually, InterpretAtMessage reads opcode_char via `get_byte()`,
    // then casts it to opcode_type*.
    // Depending on endianness and bitfield packing, this is tricky.
    // BUT, InterpretAtMessage passes `opcode` structure BY VALUE to InterpretCall.
    // In our test, we call InterpretCall directly, passing `opcode`.
    // We don't need to encode the opcode byte in bkod for THIS test function,
    // because we pass the opcode struct as argument.
    // However, InterpretCall reads from bkod immediately:
    // info = get_byte();

    std::vector<char> bytecode;

    // Function ID (info)
    append_byte(bytecode, 1); // Index 1

    // num_normal_parms
    append_byte(bytecode, 1); // 1 param

    // Normal param 1: type (1 byte), value (4 bytes)
    append_byte(bytecode, TAG_INT);
    append_blakint(bytecode, 100);

    // num_name_parms
    append_byte(bytecode, 0); // 0 params

    // Set bkod global
    test_bkod = bytecode.data();

    // Opcode struct
    opcode_type opcode;
    opcode.command = CALL;
    opcode.dest = 0;
    opcode.source1 = CALL_NO_ASSIGN;
    opcode.source2 = 0;

    object_node *o_ptr = GetObjectByID(1);
    local_var_type locals;

    // Execute
    bool result = InterpretCall(&o_ptr, 1, &locals, opcode);

    ASSERT_TRUE(result);
    ASSERT_TRUE(!g_flush_called);

    return 0;
}

static int test_InterpretCall_Overflow_Normal(void) {
    // Setup
    test_ccall_table[1] = MockCFunc;
    g_flush_called = false;

    std::vector<char> bytecode;

    // Function ID (info)
    append_byte(bytecode, 1);

    // num_normal_parms
    append_byte(bytecode, MAX_C_PARMS + 1); // OVERFLOW!

    // Should stop reading here, so no more bytes needed.

    test_bkod = bytecode.data();

    opcode_type opcode;
    opcode.command = CALL;
    opcode.source1 = CALL_NO_ASSIGN;

    object_node *o_ptr = GetObjectByID(1);
    local_var_type locals;

    // Execute
    bool result = InterpretCall(&o_ptr, 1, &locals, opcode);

    ASSERT_TRUE(!result); // Should return false
    ASSERT_TRUE(g_flush_called); // Should flush

    return 0;
}

static int test_InterpretCall_Overflow_Name(void) {
    // Setup
    test_ccall_table[1] = MockCFunc;
    g_flush_called = false;

    std::vector<char> bytecode;

    // Function ID (info)
    append_byte(bytecode, 1);

    // num_normal_parms
    append_byte(bytecode, 0);

    // num_name_parms
    append_byte(bytecode, MAX_NAME_PARMS + 1); // OVERFLOW!

    test_bkod = bytecode.data();

    opcode_type opcode;
    opcode.command = CALL;
    opcode.source1 = CALL_NO_ASSIGN;

    object_node *o_ptr = GetObjectByID(1);
    local_var_type locals;

    // Execute
    bool result = InterpretCall(&o_ptr, 1, &locals, opcode);

    ASSERT_TRUE(!result); // Should return false
    ASSERT_TRUE(g_flush_called); // Should flush

    return 0;
}

int main(void)
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_InterpretCall_Valid", test_InterpretCall_Valid, &tests_run);
    failures += run_test("test_InterpretCall_Overflow_Normal", test_InterpretCall_Overflow_Normal, &tests_run);
    failures += run_test("test_InterpretCall_Overflow_Name", test_InterpretCall_Overflow_Name, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
