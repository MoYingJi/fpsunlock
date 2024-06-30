# GI FPS Unlocker
**This small tool allows you to set a custom FPS limit for GI running under Wine**. It does not and is not supposed to work on Windows, please use any of the other existing FPS unlockers if you want that.

**The current version was tested with GI v4.5.0, and should work with future versions too**. If it breaks after an update, please [open an issue](https://codeberg.org/mkrsym1/fpsunlock/issues/new) on this repository's issue tracker.

## Usage
**The command line interface is `fpsunlock.exe [FPS]`, where**:
- `FPS` - the target framerate value (required)

Example: `fpsunlock.exe 144` - sets the limit value to 144.

**Important: You have to start it in the same Wine prefix and using the same Wine binary as the game. The game already has to be running before you start the unlocker.**

**DO NOT PUT THE FPS UNLOCKER INTO THE GAME DIRECTORY.**

## Safety
**This program is technically breaking the game's Terms of Service**, although I am not aware of any bans caused just by changing the FPS limit. **Use at your own risk.** If you somehow manage to receive a ban, please report it on the issue tracker.

## Building
To build the unlocker, you will require meson and mingw. Once all dependencies are installed, run `./build.sh` in this directory. The compiled file will be located at `build/fpsunlock.exe`.

For a debug build, run `./setup.sh` once. Then use `meson compile -C build` to compile the project.

## Mechanism
**This FPS unlocker does not inject any code into the game, and instead relies on reading and writing process memory via the WinAPI.**

The algorithm is as follows:
1. Find a process with an executable the name of which matches one of the GI executable names
2. Scan the game process memory for a pattern using `ReadProcessMemory`, then determine the FPS setter function and limit variable address using the found data
3. Change the setter function pointer to go to a nearby `ret` instruction instead of the code that changes the FPS variable
4. Set the FPS limit variable to the specified value

For more details, you can look at the code in this repository.

## Other games
**For 3rd and SR, using an unlocker program is not required. You can set the FPS values in the registry directly.**

License: MIT
