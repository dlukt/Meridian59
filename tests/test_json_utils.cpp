#include <string>
#include "test_framework.h"
#include "../blakserv/json_utils.h"

int test_json_escape(void)
{
    // Test 1: Normal string
    std::string normal = "Hello World";
    ASSERT_TRUE(JsonEscape(normal) == "Hello World");

    // Test 2: Quotes
    std::string quotes = "He said \"Hello\"";
    ASSERT_TRUE(JsonEscape(quotes) == "He said \\\"Hello\\\"");

    // Test 3: Backslashes
    std::string slashes = "C:\\Windows\\System32";
    ASSERT_TRUE(JsonEscape(slashes) == "C:\\\\Windows\\\\System32");

    // Test 4: Control characters
    std::string control = "Line1\nLine2\tTabbed";
    ASSERT_TRUE(JsonEscape(control) == "Line1\\nLine2\\tTabbed");

    // Test 5: Empty string
    ASSERT_TRUE(JsonEscape("") == "");

    // Test 6: Mixed
    std::string mixed = "{\"key\": \"value\"}";
    ASSERT_TRUE(JsonEscape(mixed) == "{\\\"key\\\": \\\"value\\\"}");

    return 0;
}
