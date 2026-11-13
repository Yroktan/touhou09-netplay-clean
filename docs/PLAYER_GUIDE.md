# Установка и игра / Installation and play

## Русский

### Установка

Нужна собственная Touhou 09 **1.50a** с неизменёнными `th09.dat` и `thbgm.dat`. Игровые архивы и музыка в релиз не входят.

Поддерживаются Steam и переносимые/не-Steam копии с совместимыми данными 1.50a. Проверки аккаунта Steam нет; оригинальный `th09.exe` для запуска мода не требуется. Репак другой версии или с изменёнными архивами нужно проверять отдельно.

1. Скачайте отдельный клиентский установщик `Touhou09-Netplay-Setup.exe` и закройте игру. Python и исходники для установки не нужны.
2. Запустите установщик, выберите папку игры кнопкой **Browse**.
3. Введите домен или IPv4 сервера, полученный от его владельца. Порт по умолчанию — `10900`.
4. Нажмите **Install**, затем запускайте **`th09-netplay.exe`**, а не оригинальный `th09.exe`.

Нажмите **Install**. Установщик сохраняет предыдущие файлы мода и настроек; игровые архивы, оригинальный EXE и сохранения не заменяются. Флажок **Launch the game after installation** запускает игру после установки. Если папка защищена Windows, запустите Setup от имени администратора.

Для ручной установки из полного пакета скопируйте `mod/th09-netplay.exe` в папку игры. Рядом создайте `.env` по образцу `.env.example` и заполните `TH09_RELAY_HOST`. Файл должен называться именно `.env`, без `.txt`. `Install.cmd` также доступен как альтернативный установщик.

### Игра через интернет

Обоим игрокам нужны одинаковая версия мода и один relay-сервер. Откройте **NETPLAY**, дождитесь **Server: Online**. Для кооператива выберите **Mode: Friends vs Boss**, затем персонажа. Создатель выбирает босса и сложность.

Первый игрок нажимает **Create Room** и передаёт четырёхзначный код. Второй выбирает **Join Room**, вводит код клавиатурой и нажимает Z/Enter; Backspace удаляет цифру. Игра начнётся после загрузки обоих клиентов. После выхода создайте новую комнату.

**Quick Match** ищет игрока с теми же режимом, боссом и сложностью. **Duel 1 vs 1** — матч до двух побед. Кооператив — отдельные бои со стандартными боссами: у каждого своё поле, здоровье босса и результат общие.

Управление: стрелки — движение, Z — огонь, X — заряд в дуэли / помощь в кооперативе, Shift — фокус. Esc — запрос общей паузы; каждый снимает свой запрос. X на паузе — выход.

### На одном компьютере

**Local Boss Co-op:** P1 — стрелки/Z/X/Shift; P2 — WASD/F/G/H. Esc — пауза, R на паузе — заново, X — главное меню. На экране результата Z/Enter повторяет бой. Для локального режима сервер и `.env` не требуются.

### Подключение

Настройки находятся в `.env` рядом с EXE:

```dotenv
TH09_RELAY_HOST=relay.example.org
TH09_RELAY_PORT=10900
```

Замените пример на адрес владельца сервера. Домен должен иметь рабочую запись IPv4. Не указывайте `https://`, путь или порт в поле хоста. После изменения перезапустите игру. Не публикуйте личный `.env`.

Если сервер недоступен, проверьте адрес, порт, интернет, VPN и разрешение исходящего UDP для игры в брандмауэре. Проброс входящих портов игрокам обычно не нужен. Если код комнаты недоступен, проверьте совпадение серверов/версий и создайте новую комнату.

## English

Supply compatible Touhou 09 **1.50a** data. Download the separate **`Touhou09-Netplay-Setup.exe`**, select your game folder, enter the relay hostname/port, and click **Install**. Python and source code are not required. Launch **`th09-netplay.exe`**. The installer writes `.env`, preserves original game files and backs up previous mod/settings files. Manual installation uses the full package's `mod/th09-netplay.exe` and a filled copy of `.env.example`.

Both players need the same release and relay. Choose **NETPLAY → Friends vs Boss → Create Room**, share the four-digit code; your friend selects **Join Room** and confirms with Z/Enter. The host chooses boss/difficulty. **Quick Match** matches the same rules. **Duel 1 vs 1** is first to two wins. Create a fresh room after leaving an online match.

Online controls: arrows/Z/X/Shift. Esc requests a shared pause; each player clears their own request. X while paused leaves. Local co-op: P1 uses arrows/Z/X/Shift, P2 uses WASD/F/G/H. Local play needs no server configuration.

Set `TH09_RELAY_HOST` to a DNS hostname with an IPv4 record or a numeric IPv4 address; `TH09_RELAY_PORT` defaults to `10900`. Do not include a URL scheme or port in the host field. Restart after changes. For connection issues check configuration, matching releases, outbound UDP, VPN and firewall settings.
