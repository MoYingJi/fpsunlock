#pragma once

#include <windows.h>
#include <stdint.h>

HANDLE open_game_process();
HMODULE get_module(HANDLE process, const wchar_t *name);
HMODULE wait_for_module(HANDLE process, const wchar_t *name);

struct fps_data {
    void *setter_ptr;
    void *fps_var;
    void *ret;
};

void find_fps_data(HANDLE process, struct fps_data *out);
