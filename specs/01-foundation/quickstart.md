# Перевірка основи — 01-foundation

Дата: 2026-10-28. Реалізовано startup-каркас, host та native target build. Фізичні H02/H03 і прийняття ітерації відкриті; окремі результати — [acceptance.md](verification/acceptance.md).

## Передумови

SDK v5.5.6 на commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, macOS arm64 host pins і Python hash-lock зафіксовано в `tools/environment/environment.lock.json`; E01 закрито для перевіреного середовища. Локальні шляхи SDK/toolchain/venv можуть зникнути: checker зупинить збірку, якщо вони недоступні. Для нових завантажень у поточному shell — `source tools/environment/python-ca.sh`.

Профіль `firmware/boards/c3-supermini-pinout-01.json`: RC GPIO3 зарезервовано, PWM GPIO0, DIR–GND на MD20A без GPIO1, живлення за D18; flash DIO 40 МГц, 4 МБ і pinned single-factory layout за D28. H02/H03 потрібні перед окремо дозволеними фізичними діями.

## Перевірка середовища

З кореня репозиторію:

```sh
export SPECIFY_FEATURE_DIRECTORY=specs/01-foundation
python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
```

Очікування: exit 0, точний SDK commit/submodule/tool list і підтверджений Python
lock replay. Відсутній lock/неправильна версія — exit 3 та blocked, не build pass.

## Host — незалежно від MCU

Використати новий каталог; для CTest — шлях із host lock:

```sh
python3 scripts/build.py host --build-dir build/host-01-new
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-01-new --output-on-failure
```

Очікування: перевірки всіх станів, retry меж 999999/1000000 мкс, п'ятої невдачі,
успіху повторної спроби, clock/protocol faults і LOW-first ordering проходять;
invalid/negative signed clock або clock-port failure до cast дає ClockFault без init/GPIO/setup;
permission завжди false. Немає SDK/GPIO/FreeRTOS у core headers/host dependencies.
За D23 запізнілий tick після deadline видає одну спробу без catch-up. Номінальний
poll 10 мс перевіряється по target config/source; гарантовану верхню межу lateness
або її фізичний вимір у 01 не заявляти. Додаткового timing-test образу не потрібно.

Окремий контрольний негативний запуск (це очікувана невдала перевірка, не build failure):

```sh
build/host-01-new/negative_probe
```

Очікування: явне повідомлення про невірне очікування й ненульовий exit. Записати
фактичний code; не стверджувати pass лише тому, що executable існує. Не маскувати
цей code у shell chain. Host suite і negative probe зберегти в `verification/host.md`.

## Target build — без підключення/прошивання

Використовуйте погоджений профіль і новий каталог збірки:

```sh
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-01-new
python3 scripts/verify/config.py --build-dir build/c3-01-new
```

Очікування: native ESP-IDF build для esp32c3, правильний flash/profile fingerprint,
окремі build та config-review результати. Invalid/unapproved profile зупиняється
до GPIO header generation/target configure. SDK graph/map не містять radios/network
компонентів та їх init/start paths. Немає automatic flash/erase/reset/monitor.
Зайнятий build directory не видаляється самостійно; використати інший новий каталог.

У generated sdkconfig перевірити log levels 0, 100 Hz FreeRTOS tick і погоджений
USB console. Один запис `ESP_WIFI_ENABLED=n` не замінює component/map/startup review.
Зберегти sdkconfig, component list, linker map, environment lock і hashes артефактів
в записах build.md/config-review.md з R/FR/SC IDs.

## GPIO-only фізичні спостереження — окремий операторський крок

Виконувати лише після окремого явного завдання на апаратні дії та H02/H03;
контактний дизайн H01 уже визначено D18–D20.
Прошивання чи підключення автоматично не дозволені цим документом.

- MD20A й насос від'єднані. Перевірити power path/bias і вимірювати тільки контакт
  PWM GPIO0 конкретної SuperMini. DIR не підключається до MCU (D20).
- До початку записати числові межі PWM LOW, похибку, bandwidth/sample interval,
  trigger/pretrigger, duration/repeats і мінімальний виявлюваний імпульс.
  Без погодженого запису H03 спостереження заблоковане.
- За cold start і ручного reset зберегти trace від подачі живлення: PWM LOW у
  межах методики, без дозволу роботи; програмного керування DIR немає.
- Окремо спостерігати вплив відсутнього/нечитаючого console host; backpressure не
  змінює retry/control behavior. За D22 передаються лише нові події одноразово;
  після drop/reconnect попередній стан не повторюється, тому відсутність виводу
  сама не є доказом firmware fault. Це правило перевіряється host-сценаріями;
  не провокувати апаратну відмову заради нового повідомлення. Для програмних fault injection користуватися host
  mocks; не додавати приховані runtime test-команди до production firmware.
- Bootloader/upload/brownout/watchdog recovery, не виконані в 01, залишити відкритими
  з власниками з [таблиці FR-013/SC-007](spec.md#власники-відкладених-startup-сценаріїв-fr-013--sc-007).
  Результат GPIO-only не доводить driver/pump safety.

Очікування: фізичний запис hardware.md із raw traces й межами методу, або чесний
blocked/not_run. Жодного твердження про нуль витрати/ізоляцію: MD20A LOW означає brake.

## Межа завершення

Порівняти фактичні записи з SC-001–SC-008. Якщо E01/H02/H03 не пройдені або
обов'язкова перевірка недоступна/невдала, відповідний критерій лишається відкритим.
SC-004 за D21 стосується SuperMini; документовані pending-дані RC/MD20A з gates
03/04/06 не блокують його в 01 й не зараховуються як прийняті інтерфейси. H02/H03
і PWM startup/reset SC-006 у 01 залишаються обов'язковими.
Відкладені Q04/Q05/Q07–Q09 не розв'язуються цим quickstart. Документи Plan,
успішна компіляція та історичні результати інших проєктів не завершують ітерацію 01.
