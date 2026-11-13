# Windows build

Python 3.11, MSVC v142 `14.29.30133`, Windows SDK `10.0.19041.0`, x86 DirectX 8 headers/libraries. The CRT is linked statically.

## Inputs

Place your own unmodified game files in `client/orig/th09_150a/`: `th09.exe`, `th09.dat`, `thbgm.dat`, `th09.cfg`. Expected hashes are in `client/orig/th09_150a.sha1`.

Prepare the Microsoft tools and original DirectX 8.1 SDK archive:

```powershell
py -3 toolchain/setup.py --accept-license --dx8-installer C:\Downloads\dx81sdk_full.exe
py -3 toolchain/setup.py --check
```

SDK/compiler binaries are private local inputs and excluded from releases. The SDK archive is extracted without executing it. The setup command requires acceptance of Microsoft's license.

## Build and configure

```powershell
py -3 -u -X utf8 scripts/build.py
py -3 -u -X utf8 scripts/build_installer.py
Copy-Item .env.example .env
```

Fill `TH09_RELAY_HOST` in your local `.env`. For the game, put a copy of this file in `game/.env`. `Build.cmd` runs the same build. The executable is written to `game/th09-netplay.exe`; original game files are not overwritten.

The second command builds the separate native Windows client installer at `installer/build/Touhou09-Netplay-Setup.exe`. Its embedded client payload is the exact game EXE built by the first command. Only developers need Python/MSVC; players run the Setup EXE.

## Run

Launch `game/th09-netplay.exe`. For online play, both players configure the same relay and use the same client version. Create a room on one computer and join it by code on the other; see [PLAYER_GUIDE.md](PLAYER_GUIDE.md).

Confirm both humans share the left field, shoot and damage native enemies; check story transitions and a completed duel with both clients before distributing a newly built EXE. Validate any forced transition checks separately from natural full-campaign completion. Package the client and matching source using [PUBLISHING.md](PUBLISHING.md).

Build and installer entry points also work from paths containing spaces. Native compile inputs use relative paths so developer home directories are not embedded through `__FILE__`.
