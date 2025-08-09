#include <windows.h>
#include <game.h>
#include <msg.h>

void unlock_fps(int32_t target_fps, int64_t interval) {
    HANDLE game = wait_for_game();
    if (game == INVALID_HANDLE_VALUE) {
        msg_err_a("Game process not found. The game has to start within 30 seconds after starting the unlocker");
    }

    uint32_t *fps = find_fps_var(game);
    if (!WriteProcessMemory(game, fps, &target_fps, sizeof(target_fps), NULL)) {
        msg_err_a("Failed to write FPS: %d", GetLastError());
    }

    if (interval > 0) {
        while (WriteProcessMemory(game, fps, &target_fps, sizeof(target_fps), NULL)) {
            Sleep(interval);
        }
    }
}

int main(int argc, char **argv) {
    int32_t target_fps;
    int64_t interval = 5000;

    switch (argc) {
        case 3:
            interval = atoi(argv[2]);

        case 2: 
            target_fps = atoi(argv[1]);
            break;

        default:
            msg_err_a("Usage: wine fpsunlock.exe [FPS] <interval>");
    }

    if (target_fps < 1) {
        msg_err_a("Invalid target FPS value");
    }

    if (interval == 0) {
        msg_err_a("Invalid interval value. Set a negative value to only write once");
    }

    unlock_fps(target_fps, interval);

    return 0;
}

