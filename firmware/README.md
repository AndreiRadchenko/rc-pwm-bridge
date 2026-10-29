# firmware

Нативний ESP-IDF каркас ітерації 01: переносиме C++11 ядро та адаптери ESP32-C3. Профіль `boards/c3-supermini-pinout-01.json`: RC GPIO3 використовується для RMT RX та GPIO ANYEDGE ISR, PWM GPIO0 статично LOW, DIR апаратно з’єднується з GND на MD20A; firmware DIR не керує. Дозвіл роботи завжди false. Реалізація03 додає захоплення одного RC50Гц: незалежна width RMT та estimated ISR timestamps, portable validation/association/fault policy, bounded latest-only diagnostics. PWM генерації, arming і мережевих сервісів немає.

Чиста target-збірка та окремий config review описані у [build.md](../specs/01-foundation/verification/build.md) і [config-review.md](../specs/01-foundation/verification/config-review.md); фізичні H02/H03 і прийняття 01 відкриті. LOW MD20A означає brake, а не відключення живлення. Прошивання й робота з навантаженням потребують окремого явного завдання.

Команди й середовище: [quickstart](../specs/01-foundation/quickstart.md).


Програмні результати03 та обмеження — [acceptance](../specs/03-rc-capture/verification/acceptance.md), команди трьох target variants/формат полів — [quickstart03](../specs/03-rc-capture/quickstart.md). Core/capture/trace — strict C++11 без SDK/I/O/heap; platform adapter — C++17 з modern RMT API ESP-IDF5.5.6. Owner priority10, diagnostics1; ресурси IDF виділяються при setup/recovery, application ISR/steady state не виділяють heap. GPIO0 застосовується LOW до запуску capture, DIR firmware не керує. Fault/resync не очищують core latch, valid лише оновлює calculated duty/last-valid=end; Q05 arming/cutoff залишається05.

Default trace/stress off. `--rc03-bench diagnostics` — record384B/100мс без trace; `--rc03-bench accuracy` — bounded128-candidate RAM window, freeze/stop, cyclic archive export після capture. Обидва bench режими не дають дозволу виходу. Фізичні accuracy/loss/latency/startup gates відкриті.
