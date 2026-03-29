#ifndef WEBHOOK_MESSAGE_H
#define WEBHOOK_MESSAGE_H

#include <string>
#include <ctime>

bool IsStructuredWebhookPayload(const std::string& message);

/**
 * Constructs a webhook payload for sending messages.
 *
 * By default, the message is escaped and wrapped in a JSON structure:
 * {"timestamp": <timestamp>, "message": "<escaped_message>"}
 * Structured webhook JSON can be passed through only when explicitly marked
 * trusted and matching the expected structured payload shape.
 *
 * @param message The input message.
 * @param timestamp The timestamp to use in the payload.
 * @param trusted_structured_json Whether the input is trusted structured JSON.
 * @return The constructed JSON payload.
 */
std::string ConstructWebhookPayload(const std::string& message, time_t timestamp, bool trusted_structured_json);
std::string ConstructWebhookPayload(const std::string& message, time_t timestamp);

#endif // WEBHOOK_MESSAGE_H
