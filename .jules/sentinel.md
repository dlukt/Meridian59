## 2024-05-23 - [Race Condition in Static Buffers]
**Vulnerability:** Found `static char` buffers used for returning strings in `GetTagName` and `GetDataName` within `blakserv/term.c`. In a multi-threaded server environment, concurrent calls to these functions would cause data corruption (race condition).
**Learning:** Legacy C code often uses static buffers to avoid dynamic allocation or caller-supplied buffers, but this pattern is inherently unsafe for threading. Even if the codebase is compiled as C++, these C-style idioms persist.
**Prevention:** Use `thread_local` (available in C++11+) for static buffers in such utility functions to ensure thread safety without changing the function signature or requiring caller changes.

## 2026-03-29 - [Webhook JSON Trust Boundary]
**Vulnerability:** `ConstructWebhookPayload` treated any message beginning with `{` as trusted JSON and returned it unescaped, allowing JSON-structure injection in outbound webhook payloads.
**Learning:** Prefix-based "looks like JSON" checks are an unsafe trust decision; log/event message content must be treated as untrusted text unless parsed and validated as a strict schema.
**Prevention:** Always build webhook envelopes server-side and JSON-escape message fields; never bypass escaping based only on first-character heuristics.
