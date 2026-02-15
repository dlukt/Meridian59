#include "test_framework.h"
#include "test_retrieve_value.h"

#include <vector>
#include <string>

// Mock dependencies by renaming them via preprocessor before inclusion
#define GetObjectByID mock_GetObjectByID
#define eprintf mock_eprintf
#define bprintf mock_bprintf
#define BlakodDebugInfo mock_BlakodDebugInfo

// Include real server headers to get types and inline functions
#include "../blakserv/blakserv.h"

// Undefine macros to allow implementing the mocks without confusion if needed,
// though we usually implement the mocked name directly.

// Mock Implementations

static object_node *g_mock_object = nullptr;

object_node * mock_GetObjectByID(int object_id) {
    if (g_mock_object && g_mock_object->object_id == object_id) return g_mock_object;
    return nullptr;
}

void mock_eprintf(const char *format, ...) {
    (void)format;
}

void mock_bprintf(const char *format, ...) {
    (void)format;
}

std::string mock_BlakodDebugInfo(void) {
    return "DebugInfo";
}

// Global variable required by inline GetKodStats in sendmsg.h
kod_statistics kod_stat;

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
    // Setup mock object
    object_node obj;
    // We need to allocate properties
    prop_type props[1];

    // Setup mock class?
    // class_node is complicated to mock fully because of dependencies,
    // but RetrieveValue only accesses o->p[data] for PROPERTY.
    // It does check GetKodStats()->debugging.

    // Initialize stats
    kod_stat.debugging = 0;

    obj.object_id = 100;
    // obj.class_id doesn't matter for PROPERTY retrieval in optimized path?
    // In RetrieveValue(int object_id...), it does:
    // o = GetObjectByID(object_id);
    // return *(val_type *)&o->p[data].val.int_val;

    // It does NOT check class_ptr for PROPERTY type.

    obj.p = props;
    props[0].id = 0;
    props[0].val.v.tag = TAG_STRING; // 2
    props[0].val.v.data = 12345;

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

    kod_stat.debugging = 0;

    obj.object_id = 200;
    obj.p = props;
    props[0].id = 0;
    props[0].val.v.tag = TAG_OBJECT; // 3
    props[0].val.v.data = 999;

    // Even if global lookup fails (g_mock_object is null), pointer access should work
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
    var_default_type vars[1];

    kod_stat.debugging = 0;

    obj.object_id = 300;
    obj.class_id = 30;
    obj.class_ptr = &cls;

    vars[0].val.v.tag = TAG_INT; // 1
    vars[0].val.v.data = 777;

    cls.class_id = 30;
    cls.class_name = (char*)"TestClass";
    cls.num_vars = 1;
    cls.vars = vars;

    g_mock_object = nullptr; // Ensure we don't use ID lookup

    local_var_type locals;
    val_type result = RetrieveValue(&obj, &locals, CLASS_VAR, 0);

    ASSERT_TRUE(result.v.tag == TAG_INT);
    ASSERT_TRUE(result.v.data == 777);

    return 0;
}

int run_retrieve_value_tests(int *tests_run, int *failures)
{
    int local_failures = 0;
    local_failures += run_test("test_retrieve_local_var", test_retrieve_local_var, tests_run);
    local_failures += run_test("test_retrieve_property_via_id", test_retrieve_property_via_id, tests_run);
    local_failures += run_test("test_retrieve_property_via_pointer", test_retrieve_property_via_pointer, tests_run);
    local_failures += run_test("test_retrieve_class_var_via_pointer", test_retrieve_class_var_via_pointer, tests_run);
    *failures += local_failures;
    return local_failures;
}
