# Relay server / Свой сервер

## Setup

The relay needs Python **3.10+**, a reachable IPv4 address and an open UDP port. It uses the standard library and does not need game assets, SDKs or administrative passwords in source code.

From the repository root:

```bash
cp deploy/.env.example .env
python3 server/relay.py
```

Server settings:

```dotenv
TH09_BIND_HOST=0.0.0.0
TH09_RELAY_PORT=10900
```

Explicit `--host` and `--port` arguments override these values. Open the selected UDP port in the provider and host firewalls. If behind NAT, forward that port to the server. Preserve existing management access when editing firewall rules.

## DNS and clients

Create an A record for a hostname you control, pointing to your relay. Test it with `nslookup YOUR_RELAY_HOSTNAME`. A hostname is an address, not protection against discovering the relay IP.

Each player sets the same hostname/port in `.env` beside `th09-netplay.exe`. Only client connection settings belong in this file. Keep SSH credentials and DNS-provider tokens separate and local.

Connect two game clients from separate computers using this hostname. Create a room in NETPLAY and join it by code from the second client; see [PLAYER_GUIDE.md](PLAYER_GUIDE.md).

For connection failures check that the relay process is running, the UDP listener, both firewalls, NAT, DNS and matching protocol versions.

## Maintenance

Keep the relay process running while players are connected. Stop it with Ctrl+C. After changing `.env` or updating the server files, run `python3 server/relay.py` again from the repository root.

Update `relay.py`, `protocol.py` and `config.py` together from one release. A restart disconnects matches and clears rooms. Protocol: 2; rules: `0x20261112`.

## Русский

Запускайте `python3 server/relay.py` из корня проекта на своём сервере. `.env` задаёт `TH09_BIND_HOST` и `TH09_RELAY_PORT`. Оставляйте процесс запущенным во время игры; остановка — Ctrl+C. После изменения настроек запустите команду снова. Откройте UDP-порт у хостинга и в брандмауэре, настройте NAT при необходимости. Создайте DNS-запись A на свой сервер и передайте игрокам домен и порт. Они вводят их при установке либо в `.env` рядом с EXE. Пароли SSH и токены DNS в репозиторий и клиентский пакет не входят.
