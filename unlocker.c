// unlocker.c - FPS unlocker refactored for native linux.
//
// To compile:
// gcc -o unlocker unlocker.c -Wall -Wextra
//
// To run without sudo (one-time setup):
// 1. Compile the program.
// 2. Grant capability: `sudo setcap cap_sys_ptrace+ep ./unlocker`
// 3. Run as a normal user.
//
// Usage: ./unlocker <PID> <FPS> [INTERVAL_MS]

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#define ANY_ (int16_t)-1

int read_process_memory(pid_t pid, uintptr_t addr, void* buf, size_t size) {
    struct iovec local = { .iov_base = buf, .iov_len = size };
    struct iovec remote = { .iov_base = (void*)addr, .iov_len = size };
    ssize_t bytes_read = process_vm_readv(pid, &local, 1, &remote, 1, 0);
    if (bytes_read == -1) {
        fprintf(stderr, "Error: Read %zu bytes at %p failed: %s\n", size, (void*)addr, strerror(errno));
        return 0;
    }
    return bytes_read == (ssize_t)size;
}

int write_process_memory(pid_t pid, uintptr_t addr, const void* buf, size_t size) {
    struct iovec local = { .iov_base = (void*)buf, .iov_len = size };
    struct iovec remote = { .iov_base = (void*)addr, .iov_len = size };
    ssize_t bytes_written = process_vm_writev(pid, &local, 1, &remote, 1, 0);
    if (bytes_written == -1) {
        fprintf(stderr, "Error: Write %zu bytes at %p failed: %s\n", size, (void*)addr, strerror(errno));
        return 0;
    }
    return bytes_written == (ssize_t)size;
}

void* find_pattern_in_process(pid_t pid, uintptr_t start, size_t len, const int16_t* pattern, size_t plen) {
    uint8_t* remote = (uint8_t*)start;
    uint8_t buf[4096];
    for (size_t i = 0; i < len; i += sizeof(buf) - plen) {
        if (!read_process_memory(pid, (uintptr_t)(remote + i), buf, sizeof(buf))) break;
        for (size_t j = 0; j < sizeof(buf) - plen; ++j) {
            int found = 1;
            for (size_t k = 0; k < plen; ++k) {
                if (pattern[k] != ANY_ && pattern[k] != buf[j + k]) { found = 0; break; }
            }
            if (found) return remote + i + j;
        }
    }
    return NULL;
}

int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) return 0;
    return !strncmp(str + len_str - len_suffix, suffix, len_suffix);
}

int verify_process(pid_t pid) {
    char path[64];
    FILE* fp;
    char cmdline[1024] = {0};

    if (pid < 1) return 0;
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    fp = fopen(path, "r");
    if (!fp) return 0;

    fread(cmdline, 1, sizeof(cmdline) - 1, fp);
    fclose(fp);

    return ends_with(cmdline, "GenshinImpact.exe") || ends_with(cmdline, "YuanShen.exe");
}

uintptr_t find_fps_var_address(pid_t pid) {
    char maps_path[64];
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    uint8_t *setter_call = NULL;
    uintptr_t fps_var_addr = 0; // Default return value is 0 (failure)

    int16_t pattern[] = { 0xB9, 0x3C, 0x00, 0x00, 0x00, 0xE8, ANY_, ANY_, ANY_, ANY_, 0x80 };
    size_t pattern_len = sizeof(pattern)/sizeof(pattern[0]);

    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    fp = fopen(maps_path, "r");
    if (!fp) {
        perror("Error: Failed to open process maps file");
        goto out;
    }

    int game_module_found = 0;
    uintptr_t last_end = 0;
    while (getline(&line, &len, fp) != -1) {
        uintptr_t start, end;
        char perms[5], path[1024] = {0};
        
        if (sscanf(line, "%lx-%lx %4s %*x %*s %*d %1023s", &start, &end, perms, path) < 3) {
            continue;
        }

        if (!game_module_found) {
            if (ends_with(path, "GenshinImpact.exe") || ends_with(path, "YuanShen.exe")) {
                game_module_found = 1;
            } else {
                continue;
            }
        }
        
        // A gap in memory addresses signifies the end of the module's segments.
        if (last_end != 0 && start != last_end) {
            break;
        }
            
        if (strchr(perms, 'x')) { // Is the section executable?
            setter_call = find_pattern_in_process(pid, start, end - start, pattern, pattern_len);
            if (setter_call) {
                break; // Pattern found, no need to scan further.
            }
        }
        last_end = end;
    }

    if (!setter_call) {
        fprintf(stderr, "Error: Could not find the FPS setter pattern.\n");
        goto out_cleanup;
    }

    uint8_t instr_bytes[7];
    uint8_t *current_addr = setter_call + 5;
    for (int i = 0; i < 5; ++i) { // Limit trace depth to prevent infinite loops
        if (!read_process_memory(pid, (uintptr_t)current_addr, instr_bytes, sizeof(instr_bytes))) {
            goto out_cleanup;
        }
        if (instr_bytes[0] == 0xE8 || instr_bytes[0] == 0xE9) { // JMP or CALL
            int32_t offset;
            memcpy(&offset, &instr_bytes[1], 4);
            current_addr += offset + 5;
        } else {
            break; // Found an instruction that is not a JMP/CALL, assume it's the target.
        }
    }

    if (instr_bytes[0] != 0x89 || instr_bytes[1] != 0x0D) {
        fprintf(stderr, "Error: Could not find the final 'mov [rip+offset], ecx' instruction.\n");
        goto out_cleanup;
    }

    int32_t rip_offset;
    memcpy(&rip_offset, &instr_bytes[2], 4);
    
    fps_var_addr = (uintptr_t)(current_addr + 6 + rip_offset);

out_cleanup:
    fclose(fp);
    free(line);
out:
    return fps_var_addr;
}

int main(int argc, char **argv) {
    pid_t pid;
    int32_t target_fps;
    int is_dry_run;
    int64_t interval = 5000;
    uintptr_t fps_addr;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <PID> <FPS> [INTERVAL_MS]\n", argv[0]);
        fprintf(stderr, "       If FPS < 1, program enters dry-run mode (read-only).\n");
        fprintf(stderr, "\nNote: Run 'sudo setcap cap_sys_ptrace+ep %s' once to run without sudo.\n", argv[0]);
        return 1;
    }

    pid = atoi(argv[1]);
    target_fps = atoi(argv[2]);
    is_dry_run = (target_fps < 1);
    if (argc == 4) {
        interval = atol(argv[3]);
    }
    
    if (!verify_process(pid)) {
        fprintf(stderr, "Error: PID %d does not appear to be a Genshin Impact process.\n", pid);
        return 1;
    }

    fps_addr = find_fps_var_address(pid);
    if (!fps_addr) {
        return 1;
    }

    if (is_dry_run) {
        int32_t current_fps;
        if (interval > 0) {
            while (read_process_memory(pid, fps_addr, &current_fps, sizeof(current_fps))) {
                printf("\rCurrent FPS limit: %-5d", current_fps);
                fflush(stdout);
                usleep(interval * 1000);
            }
            printf("\n");
        }
    } else {
        if (!write_process_memory(pid, fps_addr, &target_fps, sizeof(target_fps))) {
            return 1;
        }

        if (interval > 0) {
            while (write_process_memory(pid, fps_addr, &target_fps, sizeof(target_fps))) {
                usleep(interval * 1000);
            }
        }
    }

    return 0;
}
