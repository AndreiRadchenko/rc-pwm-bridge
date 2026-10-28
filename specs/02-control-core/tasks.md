# Завдання: переносиме ядро керування насосом

**Вхід**: [spec.md](spec.md), [plan.md](plan.md), [research.md](research.md), [data-model.md](data-model.md), [контракт](contracts/control-core.md), [quickstart.md](quickstart.md).  
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/02-control-core`  
**Стан**: всі завдання заплановані, жодна перевірка 02 не виконана.

Змінена поведінка потребує змістовних host-тестів; test-first не обов'язковий. Зберігати наявні файли й зміни. Не створювати/перемикати гілки, не комітити, не пушити, не прошивати й не керувати навантаженням автоматично. За D38 повне фізичне прийняття 01 не є умовою завершення 02; потрібні власні критерії 02 і перевірена потрібна програмна основа 01. Фізичні gates 01 та 03–06 залишаються відкритими. До 05 production arming недоступний; timeout, rearm та watchdog не визначаються тут.

Формат: `- [ ] Tnnn [P?] [USn?] дія з точним шляхом`. `[P]` означає незалежні файли й відсутність залежності від незавершених завдань; позначені завдання можуть виконуватися паралельно лише після вказаних передумов. US1–US3 мають спільне ядро і не мають паралельно змінювати одні й ті самі файли.

## Фаза 1: Підготовка

- [X] T001 Перевірити `specs/01-foundation/verification/acceptance.md`, `specs/01-foundation/verification/host.md` і `specs/01-foundation/verification/build.md` на доступність потрібної програмної основи 01; зафіксувати відповідність і відкриті H02/H03 в `specs/02-control-core/verification/prerequisites.md` (D38, R12); якщо основа відсутня або непридатна — позначити залежну роботу blocked.
- [X] T002 Перевірити тільки pinned host-інструменти й локальні шляхи за `tools/environment/environment.lock.json` через `host_tools()` із `scripts/build.py`; записати версії, результат та можливе host-блокування в `specs/02-control-core/verification/environment.md` (R03/R12). Target gate перевіряється окремо в T023; не підміняти неперевірені залежності.

## Фаза 2: Спільна основа

**Передумова**: T001 та host-частина T002; відсутність target SDK чи фізичного прийняття 01 сама по собі не блокує програмне ядро за D38. Target build потребує T023.

- [X] T003 Визначити в `firmware/components/bridge_core/include/bridge/control.hpp` C++11 value types і публічні інтерфейси для width `uint32` мкс, явного часу `uint64` мкс, validation/event status, окремих optional last-valid duty/time, disarmed/fault та effective duty/permission згідно з `specs/02-control-core/contracts/control-core.md` (R03, FR-001/003/005/006); жодних platform headers, I/O чи heap.
- [X] T004 Створити `firmware/components/bridge_core/control.cpp` зі станом startup: відсутній last-valid, disarmed, effective 0%, permission false; визначити межі update/clear без operational arming (FR-005/006).
- [X] T005 Зареєструвати `firmware/components/bridge_core/control.cpp` у `firmware/components/bridge_core/CMakeLists.txt` і `tests/host/CMakeLists.txt`; зберегти наявні startup targets та strict C++11, забезпечити окреме компонування core без platform adapter (R03).
- [X] T006 Додати незалежний public-header/link probe `tests/host/control_header_probe.cpp` та його CTest target у `tests/host/CMakeLists.txt`: компіляція strict C++11 без SDK, GPIO та FreeRTOS і без platform adapter (R03, FR-001, SC-004).

## Фаза 3: US1 — Передбачувана карта (P1)

**Мета**: отримати точну фіксовану карту лише для валідних вимірювань.  
**Незалежна перевірка**: host-перевірка шести номінальних точок, endpoints, їхніх сусідів і кожної межі округлення, без плати; команда залишається disarmed.

- [X] T007 [P] [US1] Додати в `tests/host/control_mapping_tests.cpp` очікувані 1000/1250/1400/1500/1800/2000 → 0/25/40/50/80/100%, 900/999/1000/1001/1999/2000/2001/2100, а також кожну межу округлення і сусідні значення; підтвердити незмінні endpoints після повторів (R04, FR-002/004, SC-001).
- [X] T008 [US1] Реалізувати в `firmware/components/bridge_core/control.cpp` обробку measurement із валідацією 900–2100 включно перед картою: invalid не створює valid duty і не змінює last-valid, accepted valid оновлює результат і час; на/за номінальними endpoints 0/100, усередині `(width - 1000 + 5) / 10` без underflow/overflow, без learning або фільтра; не видавати її як фактичний PWM (FR-002/004).
- [X] T009 [US1] Зареєструвати `tests/host/control_mapping_tests.cpp` у `tests/host/CMakeLists.txt` як незалежний core-only CTest; запустити host-збірку за `specs/02-control-core/quickstart.md` і CTest із фільтром назви лише цього тесту, записати результат US1 у `specs/02-control-core/verification/host.md` і зафіксувати fail/blocked за відсутності умов; фінальний повний suite повторюється в T018.

## Фаза 4: US2 — Некоректні дані та capture fault (P1)

**Мета**: відокремити invalid від valid 100%, зберегти останній valid та зафіксувати capture fault.  
**Незалежна перевірка**: host-послідовності ширин 899/900/2100/2101 і екстремальних значень, invalid після 80%, capture fault та valid під latch; порівняти статус, duty і last-valid time.

- [X] T010 [P] [US2] Додати в `tests/host/control_validation_tests.cpp` сценарії меж 899/900/2100/2101, 0 та `UINT32_MAX`, invalid-to-high regression, відсутнього valid і послідовність valid 1800 → invalid → valid 1250: duty 80% утримується до нового valid, потім стає 25%, last-valid час змінюється лише на валідних вимірюваннях; очікувати збереження розрахунку/last-valid без перетворення invalid на valid 100% (R05, FR-003/005, SC-002).
- [X] T011 [P] [US2] Додати в `tests/host/control_fault_tests.cpp` сценарії capture fault без/після valid, збереження 80%, наступний 1250 мкс → 25% і новий last-valid під latch, повторний invalid та пріоритет 0%; fault не очищується valid (D36/D37, FR-005/006, US2).
- [X] T012 [US2] Реалізувати в `firmware/components/bridge_core/control.cpp` поверх перевіреної в US1 валідації явне розрізнення invalid width і capture fault, утримання останнього валідного результату/time та latch fault згідно з `specs/02-control-core/contracts/control-core.md` (R05, FR-003/005).
- [X] T013 [US2] Зареєструвати `tests/host/control_validation_tests.cpp` і `tests/host/control_fault_tests.cpp` у `tests/host/CMakeLists.txt` як core-only CTest; перевірити traceability US2 і SC-002/003.

## Фаза 5: US3 — Безпечні межі стану (P1)

**Мета**: забезпечити нульову фактичну команду без production arming при startup, disarmed, fault і clear.  
**Незалежна перевірка**: host-послідовність startup → high valid → capture fault → clear → high valid та перевірка ефективної команди, permission і fault на кожному кроці.

- [X] T014 [P] [US3] Додати в `tests/host/control_state_tests.cpp` сценарії startup/disarmed із valid high, fault і clear із попереднім high, повторний valid; кожен стан має effective duty 0%, permission false і жодного автоматичного arming (FR-006, US3, SC-003).
- [X] T015 [P] [US3] Додати в `tests/host/control_time_tests.cpp` fake monotonic clock: зростання та рівні timestamps, invalid без зміни last-valid, регресивне measurement як `time_order_error` без мутації й регресивний capture fault із latch та effective 0%; без автоматичного timeout (FR-005/006, контракт §Межа введення).
- [X] T016 [US3] Реалізувати в `firmware/components/bridge_core/control.cpp` обробку неспадного явного часу, пріоритет capture fault при регресії, `time_order_error`, clear → disarmed і незмінні effective duty 0%/permission false; не додавати timeout/rearm (FR-005/006, Q05).
- [X] T017 [US3] Зареєструвати `tests/host/control_state_tests.cpp` і `tests/host/control_time_tests.cpp` у `tests/host/CMakeLists.txt` як core-only CTest; зберегти наявні startup тести й traceability US3/SC-003.

## Фаза 6: Інтеграція, докази та завершення

- [X] T018 Виконати host-збірку та CTest за `specs/02-control-core/quickstart.md` у новому build-каталозі; записати команди, середовище, фактичні результати всіх US1–US3 і header probe в `specs/02-control-core/verification/host.md`; помилки чи відсутні інструменти залишити fail/blocked (R03/R04/R05/R12, SC-001–004).
- [X] T023 Перевірити pinned target SDK, toolchain та replay через `scripts/environment/check.py` і `tools/environment/environment.lock.json`; записати фактичний результат target gate в `specs/02-control-core/verification/environment.md`. Відсутній target блокує T019 і завершення 02, але не host-роботу (R12).
- [X] T019 Після T018 і успішного T023 виконати native target build та config review за `specs/02-control-core/quickstart.md` без підключення або flash; підтвердити включення `control.cpp` у target component build, зафіксувати версії, команди, результати й hashes у `specs/02-control-core/verification/build.md` (R03/R12); якщо gate не пройдено — blocked/fail.
- [X] T020 [P] Записати в `specs/02-control-core/verification/hardware.md` статус not_run і власників фізичних перевірок 01 H02/H03, 03 RC, 04 PWM/DIR–GND, 05 failsafe та 06 power/pump; відокремити LOW/brake від незалежного power cutoff, не виконувати фізичні дії (R12).
- [X] T021 Перевірити в `firmware/components/bridge_core/include/bridge/control.hpp`, `firmware/components/bridge_core/control.cpp` і target config відсутність платформи в core, operational arming, радіомережі, learned endpoints та прихованого timeout; записати огляд у `specs/02-control-core/verification/config-review.md` (R03/R04/R05).
- [X] T022 Після T018–T021 зіставити R03/R04/R05/R12, FR-001–007, US1–US3 і SC-001–004 із задачами та фактичними окремими доказами в `specs/02-control-core/verification/acceptance.md`; вказати D38, критерії pass/fail/blocked, відкриті H02/H03 та відсутність дозволу роботи з насосом. Не оголошувати 02 завершеною без виконаних власних критеріїв і перевіреної програмної основи 01.

## Залежності та порядок

- Фази 1 → 2 → US1 → US2 → US3 → фаза 6; T023 має наступний вільний ID і виконується перед T019 без перенумерації наявних задач. Shared `control.cpp` та CMake змінюються послідовно; US2 використовує карту US1, US3 інтегрує стани US2. Це явно задекларовані залежності, а не незалежні релізи обладнання.
- T007 може готуватися паралельно з T008 після T003–T006; T010 і T011 — паралельно з T012 після US1; T014 і T015 — паралельно з T016 після US2. Вони змінюють різні файли; CMake-реєстрація T009/T013/T017 послідовна.
- T020 може виконуватися паралельно з T018/T019 після завершення US3: він лише документує not_run у окремому файлі. T021 потребує завершеного коду і target config; T022 потребує всіх окремих результатів, включно з blocked/not_run.
- Незалежна перевірка кожної історії використовує core-only host suite із зазначеними передумовами. Реалізація US1 без US2/US3 є лише проміжним MVP перевірки карти, не безпечним робочим контролером і не дозволом на насос.

## Стратегія виконання

Спочатку підтвердити програмну основу, host gate і portable interfaces; target gate T023 перевіряється окремо до target build. Виконувати історії послідовно й перевіряти кожну, потім зібрати окремі host/build/physical записи та порівняти їх із критеріями. За відсутності pinned інструментів або невдалих перевірок чесно зупинити залежні задачі; жодний документ чи успішний host-тест не є доказом електричного cutoff. Фізичні дії та наступні фази потребують окремого запиту користувача.
