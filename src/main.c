#include <windows.h>
#include <game.h>
#include <msg.h>
#include <bytes.h>
#include <utils.h>

int main(int argc, char **argv) {
    HANDLE game = open_game_process();

    if (!game) {
        msg_err_a("Game process not found");
    }

    uint32_t *fps = find_fps_var(game);
    
    msg_info_a("fps var: %p", fps);

    uint32_t target_fps = 144;
    if (!WriteProcessMemory(game, fps, &target_fps, sizeof(target_fps), NULL)) {
        msg_err_a("Failed to write FPS: %d", GetLastError());
    }

    msg_info_a("set fps");

    return 0;
}

