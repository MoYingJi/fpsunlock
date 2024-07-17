#pragma once

#include <windows.h>
#include <stdint.h>

HANDLE open_game_process();
HMODULE get_module(HANDLE process, const wchar_t *name);
HMODULE wait_for_module(HANDLE process, const wchar_t *name);

uint32_t *find_fps_var(HANDLE process);
