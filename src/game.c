#include <windows.h>
#include <tlhelp32.h>
#include <utils.h>
#include <msg.h>
#include <bytes.h>

#include <game.h>

static int is_game_exe(const wchar_t *exe) {
    wchar_t *exe_names[] = {
        L"GenshinImpact.exe",
        L"YuanShen.exe"
    };

    for (int i = 0; i < COUNT(exe_names); i++) {
        if (wcsicmp(exe, exe_names[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

HANDLE open_game_process() {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (!Process32FirstW(snapshot, &entry)) {
        msg_err_a("Could not enumerate processes: %d", GetLastError());
    }

    HANDLE process = INVALID_HANDLE_VALUE;
    do {
        if (is_game_exe(entry.szExeFile)) {
            process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            break;
        }
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);

    return process;
}

uint32_t *find_fps_var(HANDLE process) {
    HMODULE executable = (HMODULE)0x140000000; // not good but will work

    int16_t setter_call_pattern[] = {
        0xB9, 0x3C, 0x00, 0x00, 0x00,    // B9 3C000000    mov ecx, 60
        0xE8, ANY_, ANY_, ANY_, ANY_,    // E8 ????????    call setter
        0x80                             // 80 ?           cmp byte [x], y
    };
    uint8_t *setter_call = find_pattern_ex_in_module(process, executable, IMAGE_SCN_MEM_EXECUTE, setter_call_pattern, COUNT(setter_call_pattern));
    if (!setter_call) {
        msg_err_a("Could not find setter call");
    }

    // Traverse chain
    uint8_t bytes_at_addr[6];
    uint8_t *potential_mov = setter_call + 5;
    while (1) {
        TRY_READ_MEMORY(process, potential_mov, bytes_at_addr, sizeof(bytes_at_addr));

        if (bytes_at_addr[0] == 0xE9 || bytes_at_addr[0] == 0xE8) { // jmp / call
            potential_mov += *((int32_t*)(bytes_at_addr + 1)) + 5;
        } else {
            break;
        }
    }
    
    // 890D ????????    mov [fps], ecx
    if (bytes_at_addr[0] != 0x89 || bytes_at_addr[1] != 0x0D) {
        msg_err_a("Could not find 'mov [fps], ecx'");
    }

    int32_t fps_offset = *((int32_t*)(bytes_at_addr + 2));

    return (uint32_t*)(potential_mov + 6 + fps_offset);
}
