# Перевірка підключеної SuperMini — 2026-10-28

**Тип доказу**: board review / USB та ROM-ідентифікація; не GPIO waveform acceptance.

**Вимоги**: R01,R02,R08,R12; FR-004–FR-006,FR-012–FR-015; US1.AC1; частина SC-004.
SC-006 і решта критеріїв startup firmware цією перевіркою не закриті.

**Екземпляр**: `unit_id=c3-supermini-188b0e93032c`. Це ідентифікатор підключеної
одиниці за MAC, не затверджений variant_id/профіль розводки.

**Підключення**: користувач перед ROM-командами підтвердив тільки USB;
MD20A, насос та інші навантаження від'єднані. Порт `/dev/cu.usbmodem1101`.

**Raw докази**: [каталог сесії](board-usbmodem1101-2026-10-28/observations.json),
[команди/коди завершення](board-usbmodem1101-2026-10-28/rom-queries.json),
[manifest SHA-256](board-usbmodem1101-2026-10-28/sha256.json).
Час ROM-запитів починається 2026-10-28T12:15:22Z, reset/capture — 12:26:12Z.

## Підтверджені спостереження

| Перевірка | Фактичний результат | Межа висновку |
|---|---|---|
| USB enumeration | Espressif USB JTAG/serial debug unit, VID:PID `303A:1001`, serial `18:8B:0E:93:03:2C` | Це USB/SoC vendor, не встановлений виробник PCB. |
| ROM connection | Успішна двобічна комунікація через USB-Serial/JTAG | Native USB transport працює на цьому екземплярі; схема power/USB passives цим не встановлена. |
| SoC/package | ESP32-C3 (QFN32) | Не вгадуємо точний температурний суфікс SKU з назви SuperMini. |
| Silicon revision | v0.4 | Це ревізія кремнію, не номер ревізії PCB. |
| MAC | `18:8b:0e:93:03:2c`, збігається з USB serial | Прив'язка записів до одного екземпляра. |
| Embedded flash | 4 MB (XMC) у звіті esptool | Окремо узгоджується з JEDEC size; модель корпусу flash не визначена. |
| JEDEC flash ID | manufacturer `0x20`, device `0x4016`; detected 4 MB = 4 194 304 bytes | Не є erase/write/повним memory-health test. |
| Flash status | `0x0200` | Сирий status; без непідтвердженого декодування vendor-specific бітів. |
| Security | Secure Boot Disabled; Flash Encryption Disabled; flags `0x00000000`, crypt count `0x0` | Немає перевірених перешкод читанню цих регіонів; eFuse не записували. |
| ROM family/API | chip type ID 5, security API version 3 | ID 5 — тип, не унікальний Chip ID; для екземпляра використано MAC. |

Рядок esptool «Crystal is 40MHz» не зараховано як вимірювання: у використаній
ESP32C3 target implementation це фіксована константа. Наявність Wi-Fi/BLE у Features
описує SoC, а не доводить радіоактивність/вимкнення в майбутній firmware.

## Стан наявної flash і звичайного запуску

Без stub прочитано два регіони по 4096 байтів:

- `0x00000000`: усі байти `FF`;
- `0x00008000`: усі байти `FF`, валідну partition table в цьому регіоні не знайдено.

Обидва файли мають SHA-256
`f47a8ec3e9aff2318d896942282ad4fe37d6391c82914f54a5da8a37de1300c6`.
Інші ділянки не читалися, тому про порожність усієї flash висновку немає.

Після `hard_reset` прочитано 5 секунд console (58 036 байтів):

```text
rst:0x15 (USB_UART_CHIP_RESET),boot:0xf (SPI_FAST_FLASH_BOOT)
invalid header: 0xffffffff
```

[Повний console log](board-usbmodem1101-2026-10-28/boot-console.txt) містить повтори
повідомлення. ROM намагається SPI boot, але валідного bootloader на стандартній
адресі `0x0` у прочитаній області немає. Запуск application не спостерігався.
Це пояснює неможливість перевірити startup нашого каркаса зараз; воно не є доказом
апаратної несправності й не виправлялося автоматичним прошиванням.
Плату повернуто до звичайної спроби SPI boot; читач serial закрито.

## Інструменти й точні операції

Використано `/Users/mac/.platformio/penv/bin/python`, CPython 3.11.7, pyserial 3.5.
Фактично завантажений esptool — **4.9.0** із
`/Users/mac/.platformio/packages/tool-esptoolpy/esptool/`.
Package metadata повідомляє 5.1.0, але це не версія виконаного модуля;
path/hash/version зафіксовано в [USB inventory](board-usbmodem1101-2026-10-28/usb-enumeration.json).
Це окремий діагностичний інструмент, не підтверджений SDK environment E01.

Початковий connect використав `--before default_reset --after no_reset`.
Подальші запити: `--chip esp32c3 --baud 115200 --no-stub --before no_reset
--after no_reset` і лише `chip_id`, `flash_id`, `get_security_info`,
`read_flash_status`, `read_flash 0x0 0x1000`, `read_flash 0x8000 0x1000`.
Усі шість записаних ROM-запитів завершились exit 0. Наприкінці — `read_mac` з
`--after hard_reset` та read-only console capture, також exit 0.

`--no-stub` виключив завантаження/виконання flasher stub у RAM. Не виконувалися
write_flash, erase, write_flash_status, write_mem, eFuse burn чи наші GPIO-команди.
ROM/esptool налаштовували volatile SPI-інтерфейс для читання й виконували reset;
це не означає, що стан чипа взагалі не змінювався. Постійних flash/eFuse записів не було.
Процедури збережено поряд із raw результатами як host-side evidence scripts.

Початковий sandboxed open повернув EPERM; успішне читання виконано після дозволу
serial-доступу. Необов'язковий ioreg plist query не вдався й не використаний як доказ;
USB metadata походить із macOS enumeration через pyserial.

Довідка: [esptool reset/no-stub](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/esptool/advanced-options.html),
[основні read-команди](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/esptool/basic-commands.html).
Реальний синтаксис взято з установленого esptool 4.9.0; нові docs можуть показувати
імена через дефіс замість underscore.

## Стан на завершення ROM-сесії, до D18/D19

| Gate | Результат після цієї сесії |
|---|---|
| H01 / Q02 | SoC/package/revision, flash capacity/ID і native USB transport підтверджені. Профіль ще не погоджено: потрібні прив'язка до фізичного варіанта/фото, з'єднання контактів, power path, onboard loads, вибір RC/PWM/DIR та bias. |
| H02 | Лише USB-підключення підтверджено користувачем; напруги, схема живлення/USB+external power та зовнішній bias не вимірювалися. |
| H03 | Немає waveform/приладу/числової методики для GPIO; SC-006 залишається not_run. ROM reset log не є вимірюванням PWM/DIR. |
| H04 | RC не підключався; рівні, conditioning, timing і loss behavior не перевірялися. |
| H05 | MD20A/насос від'єднані; силові параметри, захист і незалежне аварійне відключення не перевірялися. |
| E01 | Host SDK/bootstrap lock до цієї апаратної сесії не належить; не виконано. |

Наступні апаратні докази потребують схеми/continuity й зовнішнього вимірювання:
USB-протокол не читає розводку доріжок, напругу 3V3/5V, встановлений pulldown або
імпульси на непогоджених контактах. RMT/LEDC на цьому C3 підтверджуються як SoC
можливості за документацією; фізичні capture/PWM 1000 Гц ще не виконувалися.

Ітерація 01 та SC-004 загалом не оголошені завершеними. Host-тести, target build,
реалізація firmware, flash/upload, GPIO/load acceptance у цій сесії не виконувалися.

## Подальше уточнення користувача: D18/D19

Після ROM-сесії користувач прийняв наданий pinout як достатнє джерело для
призначення контактів і визначив штатне живлення від зовнішніх 3,3 В на 3V3/GND.
У [профілі c3-supermini-pinout-01](../../../hardware/esp32-c3-supermini-profile.md)
закріплено RC=GPIO3, PWM=GPIO0, DIR=GPIO1; у 01 виходи статично LOW.
Це знімає попередню вимогу окремої схеми/continuity для H01 design та програмної
підготовки/збірки. H02/H03 залишаються окремими фізичними перевірками
напруг/bias/startup waveform. Нових вимірювань чи доступу до плати для цього
уточнення не виконували; raw докази ROM-сесії залишено незмінними.

## Подальше уточнення користувача: D20

DIR більше не належить до GPIO-профілю SuperMini: на MD20A передбачено постійну
перемичку DIR–GND. GPIO1 не використовується, firmware не керує напрямком.
Поточний профіль містить RC=GPIO3 і PWM=GPIO0. Фізичне прийняття 01 вимірює тільки
PWM на SuperMini з від'єднаними MD20A/насосом. Перевірка перемички належить 04,
фактичного напрямку насоса — 06. Це зміна дизайну, не новий вимір чи підтвердження
вже встановленої перемички; raw докази не змінювалися.

## Документальні межі інтерфейсів (T007, 2026-10-28)

RC: до підключення потрібні виміряні рівні джерела, спільна опора GND та перевірене узгодження і захист входу GPIO3. Endpoints, частота, jitter і поведінка при втраті радіо залишаються pending; власник 03, gate — перевірка до підключення та прийняття capture. Дані не є виміряними.

MD20A: за локальним datasheet rev1.0 PWM LOW задає brake; DIR передбачено постійно з'єднати з GND на драйвері за D20. Ревізія примірника, фактичні рівні, спільна земля та перемичка pending до перевірки без насоса у 04. Силове живлення, захист і незалежне аварійне відключення залишаються gate 06. Програмний LOW, зовнішній bias під час reset та незалежне відключення живлення виконують різні функції. Зовнішній flyback-діод зі схеми low-side MOSFET не переноситься на H-bridge. Див. `hardware/esp32-c3-supermini-profile.md` і локальний datasheet у `hardware/`.

Цей огляд — документальний, не фізичне прийняття RC, MD20A або SC-004; за D21 відкладені RC/MD20A поля самі не блокують SC-004 SuperMini.

## SFDP конкретної одиниці (D27–D28)

Після підтвердження умов ROM-identity підтвердила MAC `18:8b:0e:93:03:2c`, C3 rev v0.4 і XMC 4 МБ. Заголовок та базову таблицю SFDP прочитано; 24 чотирибайтні запити exit 0, raw та hashes — `sfdp-2026-10-28/`. Дані підтримують дизайн DIO 40 МГц за обґрунтуванням RCH-08; точна модель flash і фактичний boot цим не підтверджені. Були initial reset, volatile SPI operations і flash reset; запис flash/eFuse та erase не виконувалися. Плата залишена в ROM download mode. H02/H03 і SC-004 залишаються відкритими.

## Profile preflight software (T009/T010)

`python3 -m unittest discover -s tests/tooling -v`: 2 tests, exit 0. Перевірені отсутствній профіль, unapproved/conflicting pins, неправильні chip/flash, відсутні налаштування flash, DIR GPIO і погоджений дизайн із pending bench. Fixtures використовують тимчасові каталоги; відхилення до генерації header. Це software evidence, не фізична перевірка.
