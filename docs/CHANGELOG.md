# Release notes

## v1.2.0 — Story co-op

- Replaced Friends vs Boss with the original nine-stage story campaign. Both humans play together on the left field; the story CPU remains on the right.
- Removed boss selection. The host's character determines the story route.
- Added independent health, movement, shots and charge attacks for the companion. Holding shoot maintains normal fire while charging.
- Doubled CPU health from 10 to 20 and doubled the initial/phase health of boss projections on the shared field.
- Removed the arena's artificial boss respawn loop. Native dialogue, round resets, continues, story transitions and the ending are retained.
- Changed co-op networking to reliable synchronized inputs with six frames of input delay. Missing input pauses simulation. Continue-menu input is synchronized before the native menu processes it.
- Preserved the existing Duel 1 vs 1 combat and first-to-two result rules.

Both online players need the new client. Install `Touhou09-Netplay-Setup.exe` over the existing mod and select **NETPLAY → Story Co-op**. The existing relay does not need an update. The installer preserves previous mod/configuration files. Game assets, music, SDK binaries, local settings and infrastructure credentials are not included.

Validation used two real clients with equal story-world traces, independent projectiles and damage, a completed duel on the release EXE, and native installer installation/update/rollback checks. All nine campaign transitions were tested using forced round wins; this is not a claim of natural full-campaign completion for every character. Development tests, reports and logs remain outside the public package.

## v1.1.0 — Client installer

Added the standalone Windows installer, configurable relay endpoint, room codes, online duels and the original boss-arena co-op mode.
