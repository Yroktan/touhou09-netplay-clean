# Touhou 09 Netplay v1.2.0

Online duels and cooperative story campaigns for Touhou 09 1.50a. The game interface is English.

- **Story Co-op:** two humans share the left field and play the original nine-stage story campaign. The CPU remains on the right. No boss selection. CPU health is 20 instead of 10; boss projections on the shared field also have doubled health.
- **Duel 1 vs 1:** first to two wins.
- **Local Story Co-op:** the same campaign on one computer with separate controls.

Both online players must update to v1.2.0. See [release notes](docs/CHANGELOG.md).

## Install and play

1. Supply your own compatible Touhou 09 1.50a game folder.
2. Run the separate **`Touhou09-Netplay-Setup.exe`**, select the game folder, and enter the relay hostname supplied by the server owner. The default port is `10900`. Players do not need Python or source code.
3. Launch `th09-netplay.exe` from the game folder.
4. Open **NETPLAY**, choose the mode, then **Create Room**. Your friend chooses **Join Room** and enters the four-digit code.

Connection settings are stored in `.env` beside the EXE. Both players need the same mod version and relay. No server address or administrative credentials are bundled in the EXE or example configuration.

Steam and non-Steam/portable copies are supported when they supply the compatible 1.50a game data. The mod does not use Steam authentication or require the original `th09.exe` at runtime. Modified archives from other versions are not interchangeable.

## Configuration

Copy `.env.example` to `.env` and fill in the hostname. IPv4 and DNS names are supported. Process environment variables override the file. Restart the game after changing settings.

```dotenv
TH09_RELAY_HOST=relay.example.org
TH09_RELAY_PORT=10900
```

`relay.example.org` is an example, not a working game server. Keep your real `.env` local; it is excluded from Git and release packages.

## Guides

- [Players / Установка и игра](docs/PLAYER_GUIDE.md)
- [Server owners / Свой сервер](docs/SERVER_GUIDE.md)
- [Windows build](docs/BUILD_WINDOWS.md)
- [Network protocol](docs/NETPLAY.md)
- [Preparing a release](docs/PUBLISHING.md)

## Repository layout

| Path | Purpose |
| --- | --- |
| `client/` | Game source and original reconstruction tools |
| `server/` | UDP matchmaking/relay and configuration |
| `installer/` | Windows installer |
| `scripts/` | Build and release packaging |
| `deploy/` | Server configuration template |
| `docs/` | Player and maintainer guides |
| `mod/` | Verified client and standalone Setup EXE in the full release package |
| `toolchain/` | Build-tool setup scripts; proprietary SDK files are not distributed |

## Русский

Скачайте отдельный **`Touhou09-Netplay-Setup.exe`**, выберите свою Touhou 09 1.50a и укажите домен сервера от владельца. Установщик не требует Python и исходников; настройки сохранятся в `.env`. Затем запускайте `th09-netplay.exe`: **NETPLAY → Story Co-op → Create Room**. Другу передайте код; он выбирает **Join Room**. `Install.cmd` в полном исходном пакете остаётся альтернативой.

## License

GPL-3.0. Based on [Cirno-Labs/th09](https://github.com/Cirno-Labs/th09). See [LICENSE](LICENSE) and [NOTICE.txt](NOTICE.txt). Original game assets, music, SDKs and local credentials are not included.
