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
#include <cctype>

bool IsStructuredWebhookPayload(const std::string& message)
{
    static const std::string event_prefix = "{\"event\":\"";
    static const std::string params_token = "\",\"params\":{";
    size_t params_pos;
    size_t params_open_index;
    size_t params_close_index = std::string::npos;
    int depth = 0;
    bool in_string = false;
    bool escaped = false;

    if (message.rfind(event_prefix, 0) != 0)
        return false;

    params_pos = message.find(params_token);
    if (params_pos == std::string::npos)
        return false;

    params_open_index = params_pos + params_token.size() - 1; // '{' in ,"params":{
    for (size_t i = 0; i < message.size(); ++i)
    {
        char c = message[i];
        if (escaped)
        {
            escaped = false;
            continue;
        }
        if (c == '\\' && in_string)
        {
            escaped = true;
            continue;
        }
        if (c == '"')
        {
            in_string = !in_string;
            continue;
        }
        if (in_string)
            continue;
        if (c == '{')
            depth++;
        else if (c == '}')
        {
            depth--;
            if (depth < 0)
                return false;
            if (i >= params_open_index && params_close_index == std::string::npos && depth == 1)
                params_close_index = i; // closed params object; outer object still open
        }
    }

    if (in_string || depth != 0 || params_close_index == std::string::npos)
        return false;

    // After params object closes, only the outer object close is allowed.
    size_t i = params_close_index + 1;
    while (i < message.size() && isspace(static_cast<unsigned char>(message[i])))
        i++;
    if (i >= message.size() || message[i] != '}')
        return false;
    i++;
    while (i < message.size() && isspace(static_cast<unsigned char>(message[i])))
        i++;
    return i == message.size();
}

std::string ConstructWebhookPayload(const std::string& message, time_t timestamp, bool trusted_structured_json)
{
    // Allow explicit pass-through only for trusted, expected structured webhook JSON.
    if (trusted_structured_json && IsStructuredWebhookPayload(message))
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
