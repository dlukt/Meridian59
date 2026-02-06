## 2025-02-18 - Off-by-one Buffer Overflow in Channel Buffer
**Vulnerability:** Off-by-one buffer overflow in `blakserv/chanbuf.c`. `strcpy` copied `CHANBUF_SIZE` + 1 bytes (including null terminator) into a buffer of size `CHANBUF_SIZE`.
**Learning:** Checking `strlen(str) > SIZE` is insufficient when using `strcpy` because `strcpy` writes `strlen + 1` bytes. The check must be `strlen(str) >= SIZE` (to reject equal size) or use `strncpy`/`strlcpy`.
**Prevention:** Use `strlen(str) >= SIZE` or safer string copy functions like `strlcpy` or `snprintf` which handle bounds checking automatically.
