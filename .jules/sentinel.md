## 2024-05-23 - [Race Condition in Static Buffers]
**Vulnerability:** Found `static char` buffers used for returning strings in `GetTagName` and `GetDataName` within `blakserv/term.c`. In a multi-threaded server environment, concurrent calls to these functions would cause data corruption (race condition).
**Learning:** Legacy C code often uses static buffers to avoid dynamic allocation or caller-supplied buffers, but this pattern is inherently unsafe for threading. Even if the codebase is compiled as C++, these C-style idioms persist.
**Prevention:** Use `thread_local` (available in C++11+) for static buffers in such utility functions to ensure thread safety without changing the function signature or requiring caller changes.
