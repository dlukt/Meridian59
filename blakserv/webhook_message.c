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

std::string ConstructWebhookPayload(const std::string& message, time_t timestamp)
{
    if (message.empty())
    {
        return "{\"timestamp\":" + std::to_string((long)timestamp) + ",\"message\":\"\"}";
    }

    // If message already starts with '{', assume it's already JSON and skip wrapping
    if (message[0] == '{')
    {
        return message;
    }

    // Wrap plain text in JSON with timestamp
    // Use JsonEscape to prevent JSON injection and handle special characters
    std::string escaped_message = JsonEscape(message);
    return "{\"timestamp\":" + std::to_string((long)timestamp) + ",\"message\":\"" + escaped_message + "\"}";
}
