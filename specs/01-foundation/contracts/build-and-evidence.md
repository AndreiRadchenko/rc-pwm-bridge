# Контракт збірки, перевірок і доказів

Версія 3, 2026-10-28. FR-002,FR-003,FR-009–FR-016.
Наведені entry points — контракт майбутньої реалізації, зараз цих скриптів немає.

## Команди майбутнього tooling

| Точка входу | Передумови й результат |
|---|---|
| `python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json` | Перевірка SDK commit/submodules, version/hash pins і Python lock replay; не встановлює мовчазно інші версії. |
| `python3 scripts/build.py host --build-dir DIR` | Strict C++11 core + standalone tests через pinned host compiler/CMake/Ninja; SDK/GPIO не входять у host target. |
| `python3 scripts/build.py target --board-profile FILE --build-dir DIR` | E01 та матеріалізований design-approved профіль D18–D20; native idf.py build для esp32c3 з firmware root, profile preflight та generated header у DIR. |
| `python3 scripts/verify/config.py --build-dir DIR` | Generated sdkconfig, component graph, map і source/startup review; не змінює build config для «успішного» звіту. |

Exit 0 означає успіх відповідної команди; 2 — некоректний виклик, 3 — незадоволена
передумова/version/profile gate, 1 — виконана перевірка чи збірка завершилася невдало.
У жодної команди немає side effect flash/erase/reset/monitor/operate-load/commit/push.

H01 контактного дизайну визначено в [профілі](../../../hardware/esp32-c3-supermini-profile.md):
RC=GPIO3, PWM=GPIO0, DIR–GND на MD20A без GPIO1 (D20), зовнішні 3,3 В на 3V3/GND. Майбутній JSON/config
відтворює ці значення та fingerprint джерел; зараз такого файла немає.
Окрема схема/continuity та фізичні H02/H03 не є передумовами компіляції.

Для clean evidence використовують новий build directory. Якщо він уже зайнятий,
wrapper відмовляє з поясненням; самостійних rm/reset/stash/clean немає.
Підтримуваний host цього етапу — лише pinned macOS-arm64 environment з research.

## SDK/build-конфігурація

- IDF project root — `firmware`; SDK target `esp32c3`. Root CMake обмежує
  `COMPONENTS` до `main` до project.cmake. Явні залежності власних компонентів
  додаються через REQUIRES/PRIV_REQUIRES; жодного стороннього managed component у 01.
  Platform залежить лише від потрібних GPIO, esp_timer та USB Serial/JTAG driver
  компонентів і їх системних залежностей; actual graph підлягає review.
- `bridge_core` має C++11, CXX_STANDARD_REQUIRED та extensions OFF; app/adapters —
  C++17. На обох core targets явно застосовуються PRIVATE -fno-rtti/-fno-exceptions;
  actual compile options перевіряються окремо: host-core у T036, target-core у
  T022 (за compile_commands.json; export увімкнено в обох build roots). Target
  audit не є передумовою host suite; test/platform targets flags автоматично не
  успадковують. Відсутність heap-залежностей перевіряється окремо по source.
  Host і target компілюють те саме core source, не дві копії поведінки.
- GPIO та flash size/frequency/mode/partition settings походять з погодженого профілю
  та SDK design; невідомий flash не замінюється типовими 4 MB. Вибір partition layout
  не додає NVS/OTA feature; SDK-системні partition вимоги перевіряються окремо.
  Mode/frequency/partition layout зараз не вибрані; Q01/Q02 у 01 визначають і
  записують їх із джерелами до залежного target configure. Невідомі settings
  завершують preflight як blocked, а не приймаються з SDK defaults.
- SDK logs: `CONFIG_LOG_DEFAULT_LEVEL_NONE=y`, `CONFIG_LOG_MAXIMUM_EQUALS_DEFAULT=y`,
  generated levels 0. `CONFIG_FREERTOS_HZ=100` задає базовий tick 10 мс.
  За D23 nominal supervisor poll 10 мс перевіряється по config/source, а minimum
  retry pause й late tick — host-тестами. Це не доказ target lateness ≤10 мс;
  такого acceptance-критерію або окремого timing-test режиму у 01 немає.
- Local transport: `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`,
  `CONFIG_ESP_CONSOLE_SECONDARY_NONE=y`, лише після підтвердження Q02 USB routing.
  Console output у startup/retry code не використовується.
- Не підключати `esp_wifi`, `bt`, `esp_netif`, `lwip`; не викликати Wi-Fi/BT/network
  init/start. Invisible `ESP_WIFI_ENABLED` не є доказом runtime activation або
  достатнім user-settable вимикачем. Фактичний graph/map важливіші за рядок defaults.
- Application не використовує esp_restart для retries. SDK watchdog/panic/bootloader
  settings і їхні GPIO/console effects фіксуються в generated configuration;
  operational watchdog design/physical recovery належить Q05/04–06.

Перевірка відсутності radios: component/dependency list і linked archives не містять
заборонених компонентів; startup source/call review та map не показують
`esp_wifi_init`, `esp_wifi_start`, `esp_bt_controller_init`, `esp_bt_controller_enable`,
`esp_netif_init` або мережевих сервісів. Якщо заборонений dependency з'явився,
review/config check — fail, а не приховане N/A. Огляд config окремий від build pass.

## Host acceptance

Виконати state/deadline/order сценарії [startup.md](startup.md) на fake ports і
контрольованому часі. Перевірити compile/public-header isolation від SDK/FreeRTOS/GPIO.
При blocked diagnostics consumer producer не чекає, permission не змінюється.
Окремий fake-clock сценарій invalid/negative signed raw або clock-port failure
має перевірити terminal ClockFault до cast, без штучного timestamp та init/GPIO/setup.
Snapshot позначає now_valid=false і зберігає останній валідний час або 0 до першого
валідного sample; це не виміряний поточний час. Host-сценарій перевіряє цю відмінність.
За D22 сценарії діагностики мають описувати одну спробу на новий sequence,
zero/partial drop без replay на наступних polls або USB reconnect, відсутність
heartbeat у термінальному стані та одну спробу для наступної нової події.

CTest запускає справжній suite. Negative probe — окремий executable із навмисно
невірним очікуванням; прямий запуск має повернути ненульовий code. Не використовувати
лише `assert`, який може зникнути при NDEBUG. Не зараховувати compile-only чи порожній
suite. Compiler/CMake/CTest outputs і codes записуються в `verification/host.md`.
У CTest реєструється `startup_tests`; `negative_probe` запускається окремо, щоб його
навмисний failure не підміняв результат основного suite.

## Записи доказів і поточний стан

| Запис у `specs/01-foundation/verification/` | Зміст | Зараз |
|---|---|---|
| build.md | E01 closure/replay, SDK/submodules/tools/lock hashes, clean target command/config/map/artifacts | not_run / E01 відкрито; tooling/profile JSON ще не реалізовано |
| host.md | Compiler/tools/source identity, сценарії supervisor, suite й negative probe codes | not_run |
| config-review.md | Radios/dependency/map/startup і log/console review | not_run |
| board-review.md | Variant provenance, критичні поля й H01/H02/H04/H05 status | ROM/USB докази chip/revision/flash/USB; H01 design визначено D18–D20, фізичні gates відкриті |
| hardware.md | H03 numeric setup, холодний старт/reset на GPIO SuperMini; MD20A/насос від'єднані | not_run / blocked H02/H03 |

Кожен запис містить R/FR/SC/scenario IDs, expected/actual, тип доказу, дату,
виконавця, версії/конфігурацію, source fingerprint, умови й raw attachments.
Якщо Git commit ще відсутній, не вигадувати його: зберегти SHA-256 manifest джерел;
за наявності commit — commit + dirty status + manifest для незакомічених змін.

`pass` ставиться лише виконаній перевірці з результатом. Host pass не стає physical
pass, build pass не закриває H-gates. У Plan записи й raw traces не генеруються.
За D21 board-review/SC-004 для 01 розрізняє прийняті поля SuperMini та документальні
RC/MD20A pending-поля з власниками 03/04/06; останні не є блокером SC-004 у 01 або
фізичним pass своїх інтерфейсів. H02/H03 і SC-006 SuperMini залишаються обов'язковими.
При зміні profile/environment/code попередні результати залишаються історичними,
а залежні критерії потребують повторної перевірки.
