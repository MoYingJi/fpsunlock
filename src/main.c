#include <windows.h>
#include <stdio.h>
#include <game.h>
#include <msg.h>

void unlock_fps(int32_t target_fps) {
    HANDLE game = open_game_process();
    if (game == INVALID_HANDLE_VALUE) {
        msg_err_a("Game process not found. The game has to be running before you start the unlocker");
    }

    struct fps_data data;
    find_fps_data(game, &data);

    if (!WriteProcessMemory(game, data.setter_ptr, &data.ret, sizeof(data.ret), NULL)) {
        msg_err_a("Failed to patch FPS setter pointer: %d", GetLastError());
    }

    if (!WriteProcessMemory(game, data.fps_var, &target_fps, sizeof(target_fps), NULL)) {
        msg_err_a("Failed to write FPS: %d", GetLastError());
    }
}

int main(int argc, char **argv) {
    int32_t target_fps;

    switch (argc) {
        case 2: 
            target_fps = atoi(argv[1]);
            break;

        default:
            msg_err_a("Usage: wine fpsunlock.exe [FPS]");
    }

    if (target_fps < 1) {
        msg_err_a("Invalid target FPS value");
    }

    unlock_fps(target_fps);

    return 0;
}

