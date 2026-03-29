// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* webhook_message.c
*
* Logic for constructing webhook JSON payloads.
*/

#include "webhook_message.h"
#include "json_utils.h"

static bool IsStructuredWebhookJson(const std::string& message)
{
    static const std::string event_prefix = "{\"event\":\"";
    static const std::string params_token = "\",\"params\":{";
    return message.rfind(event_prefix, 0) == 0 &&
           message.find(params_token) != std::string::npos &&
           !message.empty() &&
           message.back() == '}';
}

std::string ConstructWebhookPayload(const std::string& message, time_t timestamp, bool trusted_structured_json)
{
    // Allow explicit pass-through only for trusted, expected structured webhook JSON.
    if (trusted_structured_json && IsStructuredWebhookJson(message))
    {
        return message;
    }

    std::string escaped_message = JsonEscape(message);
    return "{\"timestamp\":" + std::to_string(static_cast<long long>(timestamp)) + ",\"message\":\"" + escaped_message + "\"}";
}

std::string ConstructWebhookPayload(const std::string& message, time_t timestamp)
{
    return ConstructWebhookPayload(message, timestamp, false);
}
