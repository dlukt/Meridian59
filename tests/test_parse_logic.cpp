#include <string>
#include <vector>
#include <cstring>
#include "test_framework.h"
#include "test_parse_logic.h"

// The logic we want to test, simulating C_ParseString logic
static std::vector<std::string> ParseStringLogic(const std::string& input_str, const char* separators)
{
    std::vector<std::string> tokens;
    // Simulate the global temp string buffer (snod->data) size constraint
    const int LEN_TEMP_STRING = 1000;
    char snod_data[LEN_TEMP_STRING + 1];

    // Copy the string to parse to a local buffer, to avoid re-entrancy issues
    std::string str_to_parse = input_str;
    const char *input = str_to_parse.c_str();
    const char *token = input;

    // Skip leading delimiters
    token += strspn(token, separators);

    while (*token != '\0')
    {
        // Find end of token
        size_t token_len = strcspn(token, separators);

        // Copy token to temp string for callback
        if (token_len >= (size_t)LEN_TEMP_STRING)
            token_len = (size_t)LEN_TEMP_STRING - 1;

        memcpy(snod_data, token, token_len);
        snod_data[token_len] = '\0';

        // "Callback" - we just store the result
        tokens.push_back(std::string(snod_data));

        // Advance past token
        token += token_len;

        // Skip delimiters (strcspn stops at first delimiter, so we need to skip it/them)
        token += strspn(token, separators);
    }
    return tokens;
}

int test_parse_logic_basic(void)
{
    std::string input = "hello world";
    std::vector<std::string> expected;
    expected.push_back("hello");
    expected.push_back("world");

    std::vector<std::string> actual = ParseStringLogic(input, " ");

    ASSERT_TRUE(actual.size() == expected.size());
    for(size_t i = 0; i < expected.size(); i++) {
        ASSERT_TRUE(actual[i] == expected[i]);
    }
    return 0;
}

int test_parse_logic_multiple_delimiters(void)
{
    std::string input = "hello,,world";
    std::vector<std::string> expected;
    expected.push_back("hello");
    expected.push_back("world");

    std::vector<std::string> actual = ParseStringLogic(input, ",");

    ASSERT_TRUE(actual.size() == expected.size());
    for(size_t i = 0; i < expected.size(); i++) {
        ASSERT_TRUE(actual[i] == expected[i]);
    }
    return 0;
}

int test_parse_logic_leading_trailing_delimiters(void)
{
    std::string input = ",,hello,world,,";
    std::vector<std::string> expected;
    expected.push_back("hello");
    expected.push_back("world");

    std::vector<std::string> actual = ParseStringLogic(input, ",");

    ASSERT_TRUE(actual.size() == expected.size());
    for(size_t i = 0; i < expected.size(); i++) {
        ASSERT_TRUE(actual[i] == expected[i]);
    }
    return 0;
}

int test_parse_logic_different_delimiters(void)
{
    std::string input = "hello;world,foo bar";
    std::vector<std::string> expected;
    expected.push_back("hello");
    expected.push_back("world");
    expected.push_back("foo");
    expected.push_back("bar");

    std::vector<std::string> actual = ParseStringLogic(input, ",; ");

    ASSERT_TRUE(actual.size() == expected.size());
    for(size_t i = 0; i < expected.size(); i++) {
        ASSERT_TRUE(actual[i] == expected[i]);
    }
    return 0;
}

int test_parse_logic_empty(void)
{
    std::string input = "";
    std::vector<std::string> actual = ParseStringLogic(input, " ");

    ASSERT_TRUE(actual.size() == 0);
    return 0;
}
