#pragma once

#define COUNT(arr) (sizeof(arr) / sizeof(*arr))

#define TRY_READ_MEMORY(process, addr, buf, size) \
    if (!ReadProcessMemory(process, addr, buf, size, NULL)) { \
        msg_err_a("Failed to read process memory: %d (ReadProcessMemory @ " __FILE__ ":%d)", GetLastError(), __LINE__); \
    }
