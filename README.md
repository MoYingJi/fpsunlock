# GI FPS Unlocker
**This small tool allows you to set a custom FPS limit for GI running under Wine**. It does not and is not supposed to work on Windows, please use any of the other existing FPS unlockers if you want that.

**The current version was tested with GI v4.3.0, and should work with future versions too**. If it breaks after an update, please [open an issue](https://codeberg.org/mkrsym1/fpsunlock/issues/new) on this repository's issue tracker.

## Usage
**The command line interface is `fpsunlock.exe [FPS] <interval>`, where**:
- `FPS` - the target framerate value (required)
- `interval` - the delay between periodic writes in milliseconds (optional, default is 5000). Provide a negative value to disable periodic writes, which will cause the unlocker to exit immediatly after setting the limit once

Example: `fpsunlock.exe 144` - sets the limit value to 144 every 5 seconds.

**Important: You have to start it in the same Wine prefix and using the same Wine binary as the game. The game already has to be running before you start the unlocker.**

**DO NOT PUT THE FPS UNLOCKER INTO THE GAME DIRECTORY.**

## Safety
**This program is technically breaking the game's Terms of Service**, although I am not aware of any bans caused just by changing the FPS limit. **Use at your own risk.** If you somehow manage to receive a ban, please report it on the issue tracker.

## Mechanism
**This FPS unlocker does not inject any code into the game, and instead relies on reading and writing process memory via the WinAPI.**

The algorithm is as follows:
1. Find a process with an executable the name of which matches one of the GI executable names
2. Scan the game process memory for a pattern using `ReadProcessMemory`, then determine the FPS limit variable address using the found data
3. Periodically overwrite the FPS limit variable with the set value using `WriteProcessMemory`, or only overwrite it once if periodic writes are disabled. Periodically overwriting the variable is necessary because the game sometimes resets the limit to the one selected in it's own settings (e.g. on domain enter/leave)
4. If periodic writes are disabled or one of them fails (e.g. if the game gets closed), the unlocker will exit

For more details, you can look at the code in this repository.

## Other games
**For 3rd and SR, using an unlocker program is not required. You can set the FPS values in the registry directly.**

License: MIT
