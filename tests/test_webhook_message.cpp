#include <string>
#include <ctime>
#include "test_framework.h"
#include "../blakserv/webhook_message.h"

static int test_construct_webhook_payload_simple(void)
{
    time_t timestamp = 1234567890;
    std::string message = "Hello World";
    std::string expected = "{\"timestamp\":1234567890,\"message\":\"Hello World\"}";

    std::string result = ConstructWebhookPayload(message, timestamp);
    ASSERT_TRUE(result == expected);
    return 0;
}

static int test_construct_webhook_payload_json(void)
{
    time_t timestamp = 1234567890;
    std::string message = "{\"event\":\"test\"}";

    // Should return as-is
    std::string result = ConstructWebhookPayload(message, timestamp);
    ASSERT_TRUE(result == message);
    return 0;
}

static int test_construct_webhook_payload_injection(void)
{
    time_t timestamp = 1234567890;
    std::string message = "Hello\", \"injected\": \"value\"}";

    // Should escape quotes
    std::string result = ConstructWebhookPayload(message, timestamp);

    // Depending on JsonEscape implementation, quotes are escaped.
    // JsonEscape uses \" for quotes.
    std::string expected_message_content = "Hello\\\", \\\"injected\\\": \\\"value\\\"}";
    std::string expected = "{\"timestamp\":1234567890,\"message\":\"" + expected_message_content + "\"}";

    ASSERT_TRUE(result == expected);
    return 0;
}

static int test_construct_webhook_payload_empty(void)
{
    time_t timestamp = 1234567890;
    std::string message = "";
    std::string expected = "{\"timestamp\":1234567890,\"message\":\"\"}";

    std::string result = ConstructWebhookPayload(message, timestamp);
    ASSERT_TRUE(result == expected);
    return 0;
}

void run_webhook_message_tests(int *tests_run, int *failures)
{
    *failures += run_test("test_construct_webhook_payload_simple", test_construct_webhook_payload_simple, tests_run);
    *failures += run_test("test_construct_webhook_payload_json", test_construct_webhook_payload_json, tests_run);
    *failures += run_test("test_construct_webhook_payload_injection", test_construct_webhook_payload_injection, tests_run);
    *failures += run_test("test_construct_webhook_payload_empty", test_construct_webhook_payload_empty, tests_run);
}
