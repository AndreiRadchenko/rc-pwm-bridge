# Перевірка майбутньої основи — 01-foundation

Дата: 2026-10-28. Гілка: `01-foundation`.

Це інструкція для майбутньої реалізації. На момент Plan немає firmware build files,
реалізованих wrappers або тестового executable. Після D18–D20 контактний дизайн
профілю погоджено; його firmware JSON/config ще не реалізовано.
**Команди нижче зараз не виконувалися й ще не готові до запуску.** Тут не наведено
команд прошивання/роботи з навантаженням. Tasks/Implement потребують окремого запиту.

## Передумови

1. Прочитати [plan.md](plan.md), [research.md](research.md) й
   [build/evidence contract](contracts/build-and-evidence.md).
2. Підготувати SDK v5.5.6 саме на commit
   `880a203ae603d08a94d06aa01e0d6761958e5e9e`, його pinned submodules і інструменти
   з таблиці research. Використати CPython 3.11.7 та обраний macOS-arm64 host baseline.
   Інсталяція SDK/залежностей не виконувалася в Plan.
3. Закрити E01: повний exact-version/hash Python lock, wheelhouse і відтворення
   у другому чистому venv. До цього не заявляти reproducible target build.
   Сам bootstrap/lock tooling готується для закриття E01; залежна target-реалізація
   починається після gate. До target configure визначити й записати з джерелами
   flash mode/frequency/partition layout (Q01/Q02, 01); мовчазні defaults не приймаються.
4. Під час окремо запитаної реалізації матеріалізувати
   [профіль c3-supermini-pinout-01](../../hardware/esp32-c3-supermini-profile.md)
   у firmware JSON/config: RC=GPIO3, PWM=GPIO0, DIR–GND на MD20A без GPIO1 (D20), зовнішні 3,3 В на 3V3/GND.
   H01 design визначено D18–D20; додаткова схема/continuity не потрібна для збірки.
   H02/H03 потрібні лише перед фізичними спостереженнями.
5. Після реалізації tooling експортувати перевірені IDF_PATH/IDF_TOOLS_PATH та
   активувати саме locked SDK venv. `python3 --version`, compiler/tools versions
   мають збігатися з lock; якщо ні — зупинитися, не переписувати lock автоматично.

## Перевірка середовища

З кореня репозиторію, після появи відповідного tooling:

```sh
export SPECIFY_FEATURE_DIRECTORY=specs/01-foundation
python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
```

Очікування: exit 0, точний SDK commit/submodule/tool list і підтверджений Python
lock replay. Відсутній lock/неправильна версія — exit 3 та blocked, не build pass.

## Host — незалежно від MCU

Після реалізації host target і встановлення pinned CMake/Ninja/host compiler:

```sh
python3 scripts/build.py host --build-dir build/host-01-clean
ctest --test-dir build/host-01-clean --output-on-failure
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
build/host-01-clean/negative_probe
```

Очікування: явне повідомлення про невірне очікування й ненульовий exit. Записати
фактичний code; не стверджувати pass лише тому, що executable існує. Не маскувати
цей code у shell chain. Host suite і negative probe зберегти в `verification/host.md`.

## Target build — без підключення/прошивання

Змінна `BOARD_PROFILE` нижче має містити шлях до майбутнього JSON профілю
`c3-supermini-pinout-01`, що відтворює погоджений контактний дизайн D18–D20.
Наявність документа профілю ще не означає наявність реалізованого JSON/tooling.

```sh
python3 scripts/build.py target --board-profile "$BOARD_PROFILE" --build-dir build/c3-01-clean
python3 scripts/verify/config.py --build-dir build/c3-01-clean
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
