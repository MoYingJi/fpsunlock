#pragma once

#include <windows.h>
#include <stdint.h>

HANDLE open_game_process();
HANDLE wait_for_game();
HMODULE get_module(HANDLE process, const wchar_t *name);
HMODULE wait_for_module(HANDLE process, const wchar_t *name);

uint32_t *find_fps_var(HANDLE process);
