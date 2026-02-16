#ifndef WEBHOOK_MESSAGE_H
#define WEBHOOK_MESSAGE_H

#include <string>
#include <ctime>

/**
 * Constructs a webhook payload for sending messages.
 *
 * If the input message already appears to be JSON (starts with '{'),
 * it is returned as-is.
 * Otherwise, the message is escaped and wrapped in a JSON structure:
 * {"timestamp": <timestamp>, "message": "<escaped_message>"}
 *
 * @param message The input message.
 * @param timestamp The timestamp to use in the payload.
 * @return The constructed JSON payload.
 */
std::string ConstructWebhookPayload(const std::string& message, time_t timestamp);

#endif // WEBHOOK_MESSAGE_H
