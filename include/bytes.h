#pragma once

#include <windows.h>
#include <stdint.h>

#define ANY_ (-1)

void *find_pattern(void *address, size_t limit, int16_t *pattern, size_t length);
void *find_pattern_ex(HANDLE process, void *address, size_t limit, int16_t *pattern, size_t length);
void *find_pattern_ex_in_module(HANDLE process, HMODULE module, DWORD filter, int16_t *pattern, size_t length);
