#include "test_framework.h"
#include <string>
#include <vector>
#include <cinttypes>
#include <cstring>
#include <iostream>

// Mocking dependencies for sendmsg.h

#define _BLAKSERV_H // Prevent including the real blakserv.h

// Minimal definitions from blakserv.h
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef INT64 blak_int;

#define MAX_LOCALS 50
#define MAX_NAME_PARMS 45
#define MAX_C_PARMS 40
#define MAX_C_FUNCTION 256
#define MAX_POST_QUEUE 4000
#define MAX_DEPTH 2000

#define NIL 0
#define INVALID_ID -1
#define INVALID_CLASS -1
#define TAG_INVALID -1

enum {
   TAG_INT = 1,
   TAG_STRING = 2,
   TAG_OBJECT = 3,
   TAG_CLASS = 4,
   TAG_OVERRIDE = 14, // Made up for test if not standard
   TAG_NIL = 0 // Assuming NIL tag is 0
};

enum {
   LOCAL_VAR = 1,
   PROPERTY = 2,
   CONSTANT = 3,
   CLASS_VAR = 4
};

typedef struct
{
   INT64 data:60;
   UINT64 tag:4;
} server_constant_type;

typedef union
{
   blak_int int_val;
   server_constant_type v;
} val_type;

typedef struct
{
   blak_int value;
   int name_id;
   char type;
} parm_node;

struct class_node;

typedef struct
{
   int id;
   val_type val;
} prop_type;

typedef struct
{
   int object_id;
   int class_id;
   class_node *class_ptr;
   bool deleted;
   int garbage_ref;
   int num_props;
   prop_type *p;
} object_node;

typedef struct
{
    val_type val;
} class_var;

struct class_node
{
    int class_id;
    const char *class_name;
    const char *fname;
    int num_vars;
    class_var *vars;
    int num_properties;
    // ... minimal for test
};


// Mock global functions
int eprintf(const char *format, ...) { return 0; }
int bprintf(const char *format, ...) { return 0; }
std::string BlakodDebugInfo() { return "DebugInfo"; }

// Mock GetObjectByID
object_node *g_mock_object = nullptr;
object_node * GetObjectByID(int object_id) {
    if (g_mock_object && g_mock_object->object_id == object_id) return g_mock_object;
    return nullptr;
}

// Now include the header under test
#include "../blakserv/sendmsg.h"

// Mock GetKodStats (implemented after include so struct is defined)
kod_statistics g_kod_stats;
kod_statistics * GetKodStats(void) { return &g_kod_stats; }

// Test Functions

static int test_retrieve_local_var(void)
{
    local_var_type locals;
    locals.num_locals = 1;
    locals.locals[0].v.tag = TAG_INT;
    locals.locals[0].v.data = 42;

    val_type result = RetrieveValue(0, &locals, LOCAL_VAR, 0);

    ASSERT_TRUE(result.v.tag == TAG_INT);
    ASSERT_TRUE(result.v.data == 42);

    // Test overloaded version (should behave same)
    result = RetrieveValue((object_node*)nullptr, &locals, LOCAL_VAR, 0);
    ASSERT_TRUE(result.v.tag == TAG_INT);
    ASSERT_TRUE(result.v.data == 42);

    return 0;
}

static int test_retrieve_property_via_id(void)
{
    object_node obj;
    prop_type props[1];
    class_node cls;

    obj.object_id = 100;
    obj.class_id = 10;
    obj.class_ptr = &cls;
    obj.num_props = 1;
    obj.p = props;

    props[0].id = 0;
    props[0].val.v.tag = TAG_STRING;
    props[0].val.v.data = 12345;

    cls.class_id = 10;
    cls.num_properties = 1;

    g_mock_object = &obj;

    local_var_type locals;
    val_type result = RetrieveValue(100, &locals, PROPERTY, 0);

    ASSERT_TRUE(result.v.tag == TAG_STRING);
    ASSERT_TRUE(result.v.data == 12345);

    return 0;
}

static int test_retrieve_property_via_pointer(void)
{
    object_node obj;
    prop_type props[1];
    class_node cls;

    obj.object_id = 200;
    obj.class_id = 20;
    obj.class_ptr = &cls;
    obj.num_props = 1;
    obj.p = props;

    props[0].id = 0;
    props[0].val.v.tag = TAG_OBJECT;
    props[0].val.v.data = 999;

    cls.class_id = 20;
    cls.num_properties = 1;

    // Even if global lookup fails (g_mock_object is null/different), pointer access should work
    g_mock_object = nullptr;

    local_var_type locals;
    // Using overloaded function
    val_type result = RetrieveValue(&obj, &locals, PROPERTY, 0);

    ASSERT_TRUE(result.v.tag == TAG_OBJECT);
    ASSERT_TRUE(result.v.data == 999);

    return 0;
}

static int test_retrieve_class_var_via_pointer(void)
{
    object_node obj;
    class_node cls;
    class_var vars[1];

    obj.object_id = 300;
    obj.class_id = 30;
    obj.class_ptr = &cls;

    vars[0].val.v.tag = TAG_INT;
    vars[0].val.v.data = 777;

    cls.class_id = 30;
    cls.class_name = "TestClass";
    cls.num_vars = 1;
    cls.vars = vars;

    g_mock_object = nullptr; // Ensure we don't use ID lookup

    local_var_type locals;
    val_type result = RetrieveValue(&obj, &locals, CLASS_VAR, 0);

    ASSERT_TRUE(result.v.tag == TAG_INT);
    ASSERT_TRUE(result.v.data == 777);

    return 0;
}

int main()
{
    int tests_run = 0;
    int failures = 0;

    failures += run_test("test_retrieve_local_var", test_retrieve_local_var, &tests_run);
    failures += run_test("test_retrieve_property_via_id", test_retrieve_property_via_id, &tests_run);
    failures += run_test("test_retrieve_property_via_pointer", test_retrieve_property_via_pointer, &tests_run);
    failures += run_test("test_retrieve_class_var_via_pointer", test_retrieve_class_var_via_pointer, &tests_run);

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed.\n", failures);
        return 1;
    }

    printf("All %d tests passed.\n", tests_run);
    return 0;
}
