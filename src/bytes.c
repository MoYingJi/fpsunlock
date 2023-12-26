#include <windows.h>
#include <stdint.h>
#include <msg.h>
#include <utils.h>

#include <bytes.h>

void *find_pattern(void *address, size_t limit, int16_t *pattern, size_t length) {
    uint8_t *current = (uint8_t*)address;
    uint8_t *end = current + limit;

    size_t pattern_pos = 0;
    while (pattern_pos < length && current < end) {
        if (pattern[pattern_pos] == ANY_ || pattern[pattern_pos] == *current) {
            pattern_pos++;
            current++;
        } else if (pattern_pos != 0) {
            pattern_pos = 0;
        } else {
            current++;
        }
    }

    if (pattern_pos == length) {
        return current - length;
    } else {
        return NULL;
    }
}

void *find_pattern_ex(HANDLE process, void *address, size_t limit, int16_t *pattern, size_t length) {
    uint8_t *current = address;

    uint8_t buf[0x10000];
    size_t left = limit;

    while (left > length) {
        size_t count = left >= sizeof(buf) ? sizeof(buf) : left;
        left -= count - length;

        TRY_READ_MEMORY(process, current, buf, count);

        void *pattern_pos = find_pattern(buf, count, pattern, length);
        if (pattern_pos) {
            return current + ((intptr_t)pattern_pos - (intptr_t)buf);
        }

        current += count - length;
    }

    return NULL;
}

void *find_pattern_ex_in_module(HANDLE process, HMODULE module, DWORD filter, int16_t *pattern, size_t length) {
    uint8_t header[0x1000];

    TRY_READ_MEMORY(process, module, header, sizeof(header));

    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER*)header;
    IMAGE_NT_HEADERS64 *nt_headers = (IMAGE_NT_HEADERS64*)(header + dos_header->e_lfanew);
    IMAGE_SECTION_HEADER *section_headers = (IMAGE_SECTION_HEADER*)(nt_headers + 1);

    size_t section_count = nt_headers->FileHeader.NumberOfSections;
    for (size_t i = 0; i < section_count; i++) {
        if ((section_headers[i].Characteristics & filter) != filter) {
            continue;
        }

        void *section_addr = ((uint8_t*)module) + section_headers[i].VirtualAddress;
        void *pattern_pos = find_pattern_ex(process, section_addr, section_headers[i].SizeOfRawData, pattern, length);
        if (pattern_pos) {
            return pattern_pos;
        }
    }

    return NULL;
}
