# Задачі: 03-rc-capture

**Дата**: 2026-10-29  
**Вхідні артефакти**: [spec.md](spec.md), [plan.md](plan.md), [research.md](research.md), [data-model.md](data-model.md), [contract](contracts/rc-capture.md), [quickstart.md](quickstart.md), [checklist](checklists/capture-boundaries.md).  
**Каталог**: SPECIFY_FEATURE_DIRECTORY=specs/03-rc-capture, явно для кожного workflow script.  
**Шаблон**: .specify/templates/tasks-template.md, повернений setup-tasks.  
**Стан**: Implement — програмні задачі48/54 виконано; host18/18, tooling17/17, три target builds pass. T043–T047/T051 фізично відкриті; результати — [verification/acceptance.md](verification/acceptance.md).

Конституція v1.0.0 заповнена та ухвалена; D51–D55 визначають чинний scope/acceptance, foundation — історія. Setup-tasks повернув FEATURE_DIR саме specs/03-rc-capture та всі 4 design artifacts. Feature branch зберігається 03-rc-capture, Git mutations не потрібні.

## Формат і правила виконання

- Кожна задача: checkbox, стабільний T-ID, за потреби [P], story label для US1/US2/US3, конкретні шляхи, залежності та R/FR/SC.
- Позначка [P] дозволяє паралельну роботу лише після явно вказаних prerequisites і phase gate, у різних файлах. Це не команда запускати агентів або hardware.
- Всі три stories мають P1. Shared фази обов’язкові; complete integration delivery: US1 → US2 → US3. Незалежні host fixtures можна писати раніше за delivery своєї story після shared phase.
- Зміни control/capture/diagnostics вимагають змістовних тестів. Test-first та окреме схвалення кожного тесту не потрібні. Нижче verification tasks обов’язкові; записуються фактичні outcomes, не очікуваний pass.
- Операторські T043–T047 потребують окремого явного завдання та виконаних gates. Генерація/Implement сама по собі не дозволяє flash, підключення RC чи керування навантаженням. MD20A/насос від’єднані; GPIO0 LOW — програмний стан, не power isolation.
- Не створювати/перемикати гілки, не commit/push, не починати іншу фазу автоматично. Не змінювати prompt input, accepted core 02 API/поведінку, earlier-feature records або GPIO без належного рішення.
- Наявні файли/зміни зберігати. Нові paths нижче — заплановані, не свідчення реалізації. Умовні залежності physical evidence не блокують незалежні host/code задачі.


## Фаза 1 — Setup: вихідний стан і залежності

**Мета / критерій**: Зберегти роботу користувача, перевірити pins і визначити межі програмного/фізичного прийняття.

**Порядок**: T001 перед T002/T003; T002 і T003 незалежні.

- [X] T001 Зафіксувати початкові Git status/HEAD, наявні user changes і перелік артефактів 03 у specs/03-rc-capture/verification/acceptance.md; не змінювати, не stage і не видаляти чужі файли. (R12 / FR-005 / SC-004.)
- [X] T002 [P] Перевірити SDK/toolchain/Python/CMake/Ninja pins і локальні шляхи через scripts/environment/check.py та tools/environment/environment.lock.json; записати команди й фактичний результат у specs/03-rc-capture/verification/environment.md, при зниклих ресурсах позначити blocked без підміни версій. (Залежності: T001. R12 / FR-005 / SC-004.)
- [X] T003 [P] Звірити програмні докази 01/02, D51–D55 і checklist 31/31; записати залежності та відкриті Q03/H02/H03/прилад/receiver gate у specs/03-rc-capture/verification/prerequisites.md; Q05 залишити 05, фізичний gate не поширювати на host роботу. (Залежності: T001. R05,R12 / FR-006,FR-007 / SC-004.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Фаза 2 — Foundational: спільні межі

**Мета / критерій**: Створити лише потрібні portable/platform interfaces та host fixtures. Усі story phases починаються після завершення T001–T007.

**Порядок**: Після T004 паралельні T005/T006/T007; їхні файли різні.

- [X] T004 Оголосити у firmware/components/bridge_core/include/bridge/rc_capture.hpp стандартні C++11 DTO та policy/action interfaces: EdgeEvent, LineObservation, RxBatch/runs, PulseMeasurement, ValidationResult, CaptureState/FaultState/CaptureSnapshot, bounded capacities, units, provenance, clock/seq/epoch та поля counters/deadlines/continuity; окремий portable trace interface у firmware/components/bridge_core/include/bridge/rc_capture_trace.hpp з capacity 128/record≤48 B/header≤256 B за D55, без IDF/FreeRTOS/heap. (Залежності: T002,T003. R05,R09,R12 / FR-001–003,FR-006 / SC-001,SC-002.)
- [X] T005 [P] Визначити platform adapter lifecycle/ownership API у firmware/components/platform_esp32/include/platform/rc_capture.hpp на основі T004: один owner, setup/start/stop/recovery, RX scratch/queues, shared lock/recovery_requested та injectable boundary для fake policy tests; IDF types не переносити до portable header. (Залежності: T004. R05,R09 / FR-001,FR-003,FR-006 / SC-001,SC-002.)
- [X] T006 [P] Створити reusable fake monotonic clock, builders edge/RMT runs і scripted action/transport doubles у tests/host/rc_capture_fixtures.hpp; явні timestamps/seq/epoch, без wall-clock sleeps і MCU headers. (Залежності: T004. R05,R09,R12 / FR-001–003,FR-005 / SC-001,SC-002.)
- [X] T007 [P] Додати tests/host/rc_capture_header_probe.cpp і target до tests/host/CMakeLists.txt для самостійної компіляції capture/trace DTO headers за strict C++11 без SDK; зберегти існуючі 8 tests та negative probe. (Залежності: T004. R12 / FR-005 / SC-004.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Фаза 3 — US1: Отримати вимірювання RC (P1, MVP)

**Мета / критерій**: Незалежне прийняття: H1 fixtures дають точні status/period/start/end/RMT width; шість map points/invalid-to-high/last-valid та effective 0 перевірені host без MCU. Це input MVP, не готове hardware/iteration acceptance.

**Порядок**: Завершити shared phase; tests можна готувати поряд із implementation, без обов’язкового test-first циклу.

- [X] T008 [P] [US1] Додати fixtures включних меж width 899/900/2100/2101, period 17999/18000/22000/22001, першого unknown period, переходу 19→21 мс і D49 origin у tests/host/rc_capture_validation_tests.cpp; очікувані status і last-valid задавати незалежно від реалізації. (Залежності: T004,T006. R05,R09 / FR-001,FR-002 / SC-001.)
- [X] T009 [P] [US1] Додати нормальні edge/RMT association fixtures у tests/host/rc_capture_association_tests.cpp: levels/zero terminator, одна HIGH-пара, epoch/sequence window, independent RMT width, delayed delivery без зміни start/end та sanity 50 мкс ±1; fault cases доповнюються US2. (Залежності: T004,T006. R05,R09 / FR-001,FR-003 / SC-001.)
- [X] T010 [US1] Реалізувати bounded parser і normal-path association у firmware/components/bridge_core/rc_capture.cpp: level/duration, complete rising→falling, zero/LOW terminator, epoch/window, sanity≤50 мкс; не брати сліпо duration0 і не підбирати pulse за схожою width. (Залежності: T004,T006. R05,R09 / FR-001 / SC-001.)
- [X] T011 [US1] Реалізувати width/period validation й period origin у firmware/components/bridge_core/rc_capture.cpp: 900–2100/18000–22000 включно, перший unknown, invalid не refresh last-valid і не clamp; D49 переносить credible rising до наступного period, width не примушується до end−start. (Залежності: T010. R05,R09 / FR-001,FR-002 / SC-001.)
- [X] T012 [US1] Налаштувати modern RMT/GPIO input у firmware/components/platform_esp32/rc_capture.cpp: GPIO3 із прийнятого профілю, APB 1 МГц, filter 0, stop 5000000 ns,48 symbols/192 B, DMA/partial off; initial LOW arm/HIGH incomplete, GPIO ANYEDGE після RMT routing, estimated esp_timer timestamps та independent latest observation. (Залежності: T005,T011. R05,R09 / FR-001,FR-002,FR-006 / SC-001,SC-002.)
- [X] T013 [US1] Реалізувати final callback copy-before-reuse та ISR rearm у firmware/components/platform_esp32/rc_capture.cpp: 64 edges/8 batches, enqueue checks, epoch/arm_seq/close_seq під спільним коротким lock, stableLOW/seq/pending-edge checks навколо arm; не чекати worker/serial, normal rearm budget≤1 мс після stop. (Залежності: T012. R05,R09 / FR-001,FR-003 / SC-001,SC-002.)
- [X] T014 [US1] Скомпонувати startup 01→capture owner→ControlCore 02 у firmware/main/app_main.cpp: старт один раз після успішного setup/LOW, core calls лише owner, complete valid FIFO з estimated falling; reader unavailable не блокує capture, output GPIO0 LOW/permission false, без clear/arm/PWM. (Залежності: T011,T013. R05,R09 / FR-001,FR-002,FR-006 / SC-001,SC-002.)
- [X] T015 [US1] Додати нові sources/dependencies у firmware/components/bridge_core/CMakeLists.txt та firmware/components/platform_esp32/CMakeLists.txt; у firmware/sdkconfig.defaults встановити прийняті RMT/GPIO IRAM/cache-safe settings, IRQ priority 3 і PM/sleep off; core C++11/platform C++17, без Arduino/radio. (Залежності: T012,T013,T014. R09,R12 / FR-001,FR-005,FR-006 / SC-004.)
- [X] T016 [P] [US1] Додати reducer→core fixtures у tests/host/rc_capture_core_tests.cpp: шість точок 1000/1250/1400/1500/1800/2000→0/25/40/50/80/100, nominal endpoints/rounding regression 02, invalid-to-high, period invalid/unknown, last-valid=estimated end, initial/disarmed output 0. (Залежності: T004,T006. R05,R09,R12 / FR-001,FR-002,FR-006 / SC-001,SC-002.)
- [X] T017 [US1] Зареєструвати normal capture suites і новий portable source у tests/host/CMakeLists.txt, зберігши попередні 02 suites; portable library/header strict C++11, tests не потребують ESP32 headers. (Залежності: T008,T009,T010,T011,T016. R12 / FR-005 / SC-004.)
- [X] T018 [US1] Виконати host quickstart у новому build directory з tests/host/CMakeLists.txt, записати test list/commands/версії/результати US1 та 02 regression у specs/03-rc-capture/verification/host.md; відсутні нові suites не видавати за pass 03, target/physical ще not_run. (Залежності: T007,T008,T009,T010,T011,T012,T013,T014,T015,T016,T017. R05,R09,R12 / FR-001,FR-002,FR-005,FR-006 / SC-001,SC-002,SC-004.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Фаза 4 — US2: Відрізнити invalid від capture fault (P1)

**Мета / критерій**: Незалежне прийняття: H2/H3 fake-time/error fixtures покривають threshold/deadline/overrange/overflow/resync/barrier; invalid не latch сама, fault не очищується valid, output 0. Для інтеграції потрібна US1.

**Порядок**: T019/T020/T021 можна написати незалежно після shared phase; source changes T022–T027 серіалізовані.

- [X] T019 [P] [US2] Додати tests/host/rc_capture_fault_tests.cpp: короткі/довгі/неповні pulses, expected HIGH stop 5 мс без false falling, unexpected truncation, saturated48 symbols, missing/repeated/reordered edges, queue drop та invalid period без fault сам по собі; resource guard — окрема причина. (Залежності: T004,T006. R05,R09 / FR-002,FR-003 / SC-001,SC-002.)
- [X] T020 [P] [US2] Додати tests/host/rc_capture_time_tests.cpp: startup HIGH/LOW;100000 мкс threshold±1, deliveryдо 110000, ended episode 105 мс/owner 109 мс, missed deadline/lateness, guard 12000+10000 мкс/final reset, processing 10000 мкс, clock regression, час через 2³²мкс і sequence wrap. (Залежності: T004,T006. R05,R09 / FR-003,FR-006 / SC-001,SC-002.)
- [X] T021 [P] [US2] Додати tests/host/rc_capture_adapter_tests.cpp через fake boundary: concurrent edge/monitor/arm/recovery schedules, sticky error поза повною чергою, masked observation gap, recovery under LOW, barrier/discard count, valid під latch; fault injection на кожному init кроці RMT/GPIO/timer/task, partial-init cleanup лише власних ресурсів, retry≥100 мс без дубльованих handles/tasks/ISR, LOW/fault до та після успішного retry. Fake boundary перевіряє lifecycle contract, не реальну ISR latency чи фактичний SDK cleanup. (Залежності: T004,T006. R05,R09,R12 / FR-001,FR-003,FR-006 / SC-001,SC-002.)
- [X] T022 [US2] Доповнити firmware/components/bridge_core/rc_capture.cpp classification/recovery policy: invalid не latch; genuine boundary loss/API/drop/saturation дає sticky reason+counter/discard/resync; expected HIGH stop incomplete, без вигаданого end, два нові rising після втрати меж. (Залежності: T011. R05,R09 / FR-002,FR-003 / SC-001,SC-002.)
- [X] T023 [US2] Реалізувати monitor/guards у firmware/components/bridge_core/rc_capture.cpp: common 100 мс від last change/startup, pending completed episodes не стираються новим edge, coherent seq recheck, delivery≤110 мс, missed deadline/lateness, guard 12+10 мс і processing 10 мс; final будь-якого типу очищує guard. (Залежності: T022. R05,R09 / FR-003 / SC-002.)
- [X] T024 [US2] Додати fault barrier/монотонну серіалізацію owner у firmware/main/app_main.cpp та firmware/components/platform_esp32/rc_capture.cpp: fault(now), discard/count pre-barrier samples без зміни end, newer valid під latch, TimeOrderError→fault, усі effective 0/permission false. (Залежності: T014,T022,T023. R05,R09 / FR-003,FR-006 / SC-001,SC-002.)
- [X] T025 [US2] Реалізувати overflow/error ingress і recovery у firmware/components/platform_esp32/rc_capture.cpp: sticky metadata поза queues, saturating counters, GPIO interrupt mask при overflow, recovery_requested блокує ISR rearm; abort/reset/re-enable у LOW, continuity false на невидимий gap, новий interval без fake edge. (Залежності: T013,T023,T024. R05,R09 / FR-003,FR-006 / SC-002.)
- [X] T026 [US2] Під’єднати esp_timer notify 1000 мкс і owner priority 10 у firmware/components/platform_esp32/rc_capture.cpp: bounded decode/poll, coherent observation, timer без core/I/O, облік фактичного threshold/reported/completion/arm time та дотримання 10 мс delivery/processing budget. (Залежності: T023,T025. R05,R09 / FR-003,FR-006 / SC-002.)
- [X] T027 [US2] Завершити init/resource lifecycle у firmware/components/platform_esp32/rc_capture.cpp та firmware/main/app_main.cpp: partial-init cleanup, owner-only resource retry≥100 мс, LOW/fault при API failure, повторний setup не дублює tasks/timer/ISR і не змінює retry policy 01 або Q05 watchdog. (Залежності: T024,T025,T026. R05,R09 / FR-003,FR-006 / SC-002.)
- [X] T028 [US2] Зареєструвати fault/time/adapter suites у tests/host/CMakeLists.txt без ESP-IDF залежностей; core 02 suites і portable header probe залишити активними. (Залежності: T019,T020,T021,T022,T023,T024,T025,T026,T027. R12 / FR-005 / SC-004.)
- [X] T029 [US2] Запустити meaningful host suites US1/US2/02, записати факти й failed/not_run cases у specs/03-rc-capture/verification/host.md; окремо показати invalid не refresh, latch не auto-clear, ended stuck і masked gap/clock/error scenarios. (Залежності: T028. R05,R09,R12 / FR-001–003,FR-005,FR-006 / SC-001,SC-002,SC-004.)
- [X] T030 [US2] Додати bounded RAM timing instrumentation у firmware/components/platform_esp32/rc_capture.cpp та firmware/components/platform_esp32/include/platform/rc_capture.hpp: max rearm/processing/fault lateness, dropped/duplicate counts, без per-edge serial/heap history; інтерфейс знімка для майбутніх physical checks без нового GPIO. (Залежності: T026,T027,T029. R09,R12 / FR-003,FR-005 / SC-002,SC-003.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Фаза 5 — US3: Діагностика та докази (P1)

**Мета / критерій**: Незалежне прийняття policy: H4 із fake clock/reader. Повне прийняття story додатково потребує B1 build та P1/P2 hardware evidence з US1/US2; host не доводить timing/електрику.

**Порядок**: T031/T032 незалежні після shared phase; T040 та T042 незалежні після своїх prerequisites. T043–T047 — операторські, автоматично не виконуються.

- [X] T031 [P] [US3] Додати tests/host/rc_diagnostics_policy_tests.cpp із fake clock: spacing 99999/100000 мкс/max 10/с, latest overwrite/version ack під concurrent publish, окремі overwritten_snapshots/busy_slots/write_drops, no doublecount, saturation/reset і відсутність зміни core permission/fault від reader. (Залежності: T004,T006. R09,R11 / FR-004,FR-006 / SC-003,SC-004.)
- [X] T032 [P] [US3] Додати tests/host/rc_diagnostics_transport_tests.cpp: busy/no reader/reconnect, максимум один committed record, zero/partial write без retry remainder/history burst, newline+rc03 framing, усі max fields≤384 B і bench padding рівно 384 B. (Залежності: T004,T006. R09,R11,R12 / FR-004,FR-005 / SC-003,SC-004.)
- [X] T033 [US3] Пов’язати CaptureSnapshot із DiagnosticSnapshot у firmware/components/platform_esp32/include/platform/startup_port.hpp: RCmeasurement/status/start/end/period, calculated/effective/permission, reason/counters/session і startup fields; переоцінити існуючий sizeof≤96 bound на явний обґрунтований RAM bound, зберегти переносимість header. (Залежності: T004,T030. R09,R11 / FR-004,FR-006 / SC-003.)
- [X] T034 [US3] Адаптувати переносиму DiagnosticConsumer/mailbox policy у firmware/components/platform_esp32/startup_policy.cpp і firmware/components/platform_esp32/include/platform/startup_port.hpp до D43 для композиції 03: latest/version-specific ack, slots≥100 мс, окремі skip/drop/overwrite units, fixed format≤384 B, без command/reset source; зберегти startup retry/LOW 01 і відокремити legacy D22 test context від D43, щоб старий одноразовий consumer не задавав політику 03. (Залежності: T033. R09,R11 / FR-004,FR-006 / SC-003,SC-004.)
- [X] T035 [US3] Переробити sole USB writer у firmware/components/platform_esp32/local_diagnostics.cpp та firmware/components/platform_esp32/include/platform/local_diagnostics.hpp: diagnostics priority 1, TX 512 B, wait_tx_done(0) gate і одне write(...,0), one in-flight, drop без retry, reconnect latest у звичайному темпі; is_connected не трактувати як open reader. (Залежності: T034. R09,R11 / FR-004 / SC-003.)
- [X] T036 [US3] Під’єднати owner publication у firmware/main/app_main.cpp та firmware/components/platform_esp32/rc_capture.cpp: один актуальний snapshot зі sticky capture/core state й counters; форматування/USB тільки worker, друк не затримує capture/core, production clear/arming відсутні. (Залежності: T030,T033,T035. R05,R09,R11 / FR-004,FR-006 / SC-002,SC-003.)
- [X] T037 [US3] Додати default-off compile-time bench опцію у firmware/main/Kconfig.projbuild і firmware/components/platform_esp32/local_diagnostics.cpp; fragment firmware/sdkconfig.rc03-bench.defaults та явний target-only flag у scripts/build.py для поєднання baseline+bench defaults без зміни baseline: актуальний record доповнюється пробілами до 384 B включно із framing, спроби≥100 мс, значення capture не підміняються; default не вмикає bench, runtime commands не додаються. (Залежності: T035,T036. R09,R11,R12 / FR-004,FR-005 / SC-003.)
- [X] T052 [P] [US3] Додати tests/host/rc_capture_trace_tests.cpp для D55 collector:128 consecutive candidates, unknown/invalid зберігаються, нормальний full→freeze без overwrite, незмінність dump, baseline/delta без reset latch/counters, reader-independent capture, gap/saturation/fault не приховуються, межі record≤48 B/header≤256 B/total≤6400 B. (Залежності: T004,T006. R09,R12 / FR-001,FR-005,FR-006 / SC-003,SC-004.)
- [X] T053 [US3] Реалізувати portable collector у firmware/components/bridge_core/rc_capture_trace.cpp та firmware/components/bridge_core/include/bridge/rc_capture_trace.hpp, owner integration у firmware/components/platform_esp32/rc_capture.cpp, default-off trace flag у firmware/main/Kconfig.projbuild, firmware/sdkconfig.rc03-bench.defaults та scripts/build.py і frozen archive exporter у firmware/components/platform_esp32/local_diagnostics.cpp; 128×≤48 B+≤256 B, capture без I/O, freeze/stop перед cyclic HEADER/records/END export≤10frames/с/≤384 B/one in-flight; raw значення не коригувати, production latest-only незмінний, output LOW/latch незмінні, окремий операторський restart для нового window. (Залежності: T030,T037,T052. R05,R09,R11,R12 / FR-001,FR-004–006 / SC-002–004.)
- [X] T054 [P] [US3] Реалізувати offline comparator scripts/verify/rc_capture_trace.py і fixed reference fixtures у tests/tooling/test_rc_capture_trace.py: повний archive/version/count/order/correspondence, normalized CSV pulse_start_ns/pulse_end_ns, per-sample width та rising-interval E, direct 5/10 мкс з U≤1 мкс окремо, Decimal без rounding/average/offset/drift adjustment, complete/failed/blocked report і source hashes; partial/cross-session cycles, gaps, несумісні duplicate records та невідоме U не pass. Виконати Python tooling tests; це synthetic evidence, не фізичне прийняття. (Залежності: T004,T006. R09,R12 / FR-001,FR-005 / SC-003,SC-004.)

- [X] T038 [US3] Зареєструвати diagnostics/trace suites, rc_capture_trace.cpp та fake transport у tests/host/CMakeLists.txt; platform policy host compile C++17 лишити без IDF, portable capture/core strict C++11; зберегти startup init/LOW regression 01 у tests/host/startup_tests.cpp; expectations, які D43 явно замінив у 03, покрити новими D43 tests, не послаблювати незмінні safety assertions. (Залежності: T031,T032,T033,T034,T035,T036,T037,T052,T053,T054. R12 / FR-005 / SC-004.)
- [X] T039 [US3] Виконати повний applicable host набір 03 та 01/02 regressions у новому build directory і записати test list/commands/результати в specs/03-rc-capture/verification/host.md; не переносити runtime/physical claims із fake tests. (Залежності: T038. R05,R09,R11,R12 / FR-001–006 / SC-001–004.)
- [X] T040 [US3] Розширити scripts/verify/config.py і tests/tooling/test_build_contract.py для effective C3/RMT/IRAM/cache-safe/PM/radios settings та source/component graph 03, normal/diagnostics-stress/accuracy-trace flag routing у scripts/build.py, production trace off і trace objects лише у відповідному graph; fixtures мають відхиляти відсутній capture object/неправильний cfg, не лише шукати рядки в source; виконати tooling tests і записати у specs/03-rc-capture/verification/config-review.md. (Залежності: T015,T037,T039. R09,R12 / FR-005,FR-006 / SC-004.)
- [X] T041 [US3] Виконати pinned native target build без плати через scripts/build.py і scripts/verify/config.py у новому directory, включно з normal, diagnostics-stress без trace та accuracy-trace+stress configs; перевірити map/graph, весь ISR call graph/internal data placement, static RAM/stacks/allocation boundaries й hashes; записати команди та факти у specs/03-rc-capture/verification/build.md і specs/03-rc-capture/verification/config-review.md, latency не оголошувати доведеною. (Залежності: T002,T030,T037,T040. R09,R12 / FR-005,FR-006 / SC-004.)
- [X] T042 [P] [US3] Підготувати specs/03-rc-capture/verification/hardware-procedure.md і specs/03-rc-capture/verification/hardware.md як not_run: Q03/H02/H03/інструмент gates, native bench build/flash команди лише для окремого доручення, profile GPIO3/GPIO0, MD20A/насос від’єднані, D53 scope, D54 direct E/U rule, required raw trace/counters/latencies, D55 короткі accuracy windows/export/offline compare та окремі 600 с loss/timing режими. (Залежності: T003,T030,T037,T039. R05,R09,R11,R12 / FR-003–007 / SC-002–004.)
- [ ] T043 [US3] [ОПЕРАТОР: окреме завдання] Підтвердити придатність стенда за specs/03-rc-capture/verification/hardware-procedure.md: дійсні H02/H03, перевірене пониження HJ до 3,3 В/ground/protection до підключення, analyzer≥10 MS/s та обґрунтовані U_width/U_interval≤1 мкс; записати evidence в specs/03-rc-capture/verification/hardware.md, за відсутності даних лишити задачу unchecked/blocked. (Залежності: T041,T042. R12 / FR-005,FR-007 / SC-003,SC-004.)
- [ ] T044 [US3] [ОПЕРАТОР: окреме завдання на flash/підключення] Розгорнути точний build із specs/03-rc-capture/verification/build.md за specs/03-rc-capture/verification/hardware-procedure.md, перевірити GPIO0 LOW/effective 0/permission false, підключити HJ 50 Гц та analyzer після conditioning лише за виконаних gates; firmware hash/config/умови записати у specs/03-rc-capture/verification/hardware.md. (Залежності: T043. R05,R12 / FR-006,FR-007 / SC-002,SC-004.)
- [ ] T045 [US3] [ОПЕРАТОР: окреме завдання] Виконати HJ sweep короткими 128-record accuracy windows за D55 під max diagnostics при active/slow/absent reader, після freeze/stop зберегти archive й analyzer CSV та обчислити кожне E через scripts/verify/rc_capture_trace.py; окремо startup/stuck HIGH/LOW/missing-pulse перевірки з specs/03-rc-capture/verification/hardware-procedure.md; E_width кожного імпульсу≤5 мкс/E_interval кожного інтервалу≤10 мкс, U≤1 мкс окремо без запасу; rearm≤1 мс після stop, processing≤10 мс, fault 100..110 мс, без automatic clear; expected/measured/raw trace/latencies записати у specs/03-rc-capture/verification/hardware.md, точні 18/22 мс та складні malformed лишити host за D53. (Залежності: T044. R05,R09,R12 / FR-001–003,FR-005–007 / SC-001–004.)
- [ ] T046 [US3] [ОПЕРАТОР: окреме завдання] З accuracy trace off провести окремі 600 с вікна 50 Гц з bench 384 B/100 мс за active/slow/absent reader; звірити correct analyzer count з matched/accepted valid,0 втрат/duplicates, не виключати rearm gaps; actual payload/attempts/drops і max latencies записати у specs/03-rc-capture/verification/hardware.md, output LOW увесь час; не заявляти per-pulse accuracy всіх 600 с на підставі counters/maxima. (Залежності: T044,T045. R09,R11,R12 / FR-004–007 / SC-003,SC-004.)
- [ ] T047 [US3] [ОПЕРАТОР: окреме завдання] Виконати reader reconnect/partial transmission/fault під max diagnostics за specs/03-rc-capture/verification/hardware-procedure.md: максимум один committed record потім latest без burst, counters/flags не стираються, core fault deadline незалежний від тексту й output LOW; вимірювання та невиконані cases записати у specs/03-rc-capture/verification/hardware.md. (Залежності: T045,T046. R05,R09,R11,R12 / FR-003–007 / SC-002–004.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Фаза 6 — Документація та підсумкове прийняття

**Мета / критерій**: Зберегти повний R/FR/SC→tasks→evidence зв’язок; не оголошувати iteration accepted із невиконаною physical частиною.

**Порядок**: T048 після готових build/procedure може йти паралельно з операторською чергою; T049/T050 можна закінчити з чесним provisional/not_run, T051 — лише з повними доказами.

- [X] T048 [P] Уточнити фактичні команди/імена suites/трьох build variants, RAM trace export/offline comparison у specs/03-rc-capture/quickstart.md, tests/README.md, firmware/README.md та scripts/README.md; задокументувати межі переносимого/IDF коду, diagnostic format/counters і незмінні Q05/no arming, без твердження hardware pass. (Залежності: T039,T041,T042. R09,R11,R12 / FR-004–007 / SC-004.)
- [X] T049 Звести актуальні host/tooling/build evidence й source/config/artifact hashes у specs/03-rc-capture/verification/acceptance.md; physical записи долучати лише якщо фактично отримані, інакше not_run/blocked; при зміні зібраного коду повторювати лише зачеплені перевірки, не видавати старий hash за новий результат. (Залежності: T039,T040,T041,T048. R12 / FR-005,FR-007 / SC-004.)
- [X] T050 Виконати cross-artifact/constitution traceability review у specs/03-rc-capture/verification/acceptance.md: усі R/FR/SC→T→host/build/physical, D51–D55/checklist 31/31, one input/native IDF/no arming/LOW, no endpoint learning, Q03/Q05/Q06 owners; відкриті physical tasks залишити відкритими. (Залежності: T049. R05,R09,R11,R12 / FR-001–007 / SC-001–004.)
- [ ] T051 Після всіх обов’язкових програмних і фізичних критеріїв записати висновок прийняття 03 у specs/03-rc-capture/verification/acceptance.md; незадоволені допуски/невідоме U/відкриті gates блокують завершення, receiver gate до використання зберігається; без автоматичного Plan/Implement наступної ітерації чи commit/push. (Залежності: T018,T029,T039,T041,T043,T044,T045,T046,T047,T050. R05,R09,R11,R12 / FR-001–007 / SC-001–004.)

**Checkpoint**: завершення code/test задач не є фізичним прийняттям; непроведене залишається unchecked/not_run.

## Стабільні IDs після analyze-guided

T052–T054 додано у логічному місці фази US3, не перенумеровуючи T001–T051 та їхні посилання у попередніх звітах. Числовий ID більше не задає сам виконання: T038 чекає T052–T054, далі T039/build/операторські кроки. Залежності утворюють DAG без циклів. Це додані coverage tasks для C1, не новий user story.

## Залежності та граф виконання

~~~mermaid
flowchart TD
  S[Setup T001–T003] --> F[Shared T004–T007]
  F --> U1[US1 T008–T018]
  U1 --> U2[US2 T019–T030]
  U2 --> U3[US3 policy/host T031–T039 + T052–T054]
  U3 --> V[Config/build T040–T041]
  U3 --> P[Operator procedure T042]
  V --> G[Electrical/metrology gate T043]
  P --> G
  G --> H[Operator T044–T047]
  V --> D[Docs/evidence review T048–T050]
  P --> D
  H --> A[Acceptance T051]
  D --> A
~~~

Граф показує delivery gates; індивідуальні prerequisites у задачах точніші. T049/T050 можуть зафіксувати provisional software status до фізичних перевірок, але не замінюють T043–T047 і не дозволяють закрити T051. Немає залежності host/code від майбутнього приймача, Q05 cutoff або насоса.

### Паралельні приклади за stories

- **Shared**: після T001 — T002(environment.md) + T003(prerequisites.md); після T004 — T005(platform header) + T006(fixtures header) + T007(header probe/CMake).
- **US1**: T008(validation_tests.cpp) + T009(association_tests.cpp) + T016(core_tests.cpp) після shared phase. У той самий час T010/parser може змінювати інший source. T010/T011 і T012/T013 послідовні через спільні файли.
- **US2**: T019(fault_tests.cpp) + T020(time_tests.cpp) + T021(adapter_tests.cpp) після shared phase. Це тести договору, не три паралельні редактори rc_capture.cpp.
- **US3**: T031(policy_tests.cpp) + T032(transport_tests.cpp) + T052(trace_tests.cpp) + T054(offline Python comparator/tests), після shared phase. T053 змінює platform/export sources послідовно після T037. Після потрібних залежностей T040(config tooling) + T042(hardware procedure) не мають shared files.
- **Фінал**: T048 лише docs може йти поряд із затвердженою операторською чергою; однак writers specs/03-rc-capture/verification/hardware.md у T043–T047 завжди послідовні.

CMake/shared headers/app/source tasks не запускати паралельно з іншими writers тих самих файлів. [P] не змінює order/dependencies і не є дозволом на hardware.

## Простежуваність і критерії

| Вимоги / критерії | Основні задачі | Перевірка / запис |
|---|---|---|
| R05 / FR-001,FR-002 / US1, SC-001 | T004,T008–T018 | Host H1/association/core; verification/host.md |
| R05,R09 / FR-003,FR-006 / US2, SC-001/002 | T019–T030 | H2/H3; last-valid/fault/disarmed; host.md, physical latencies T045 |
| R09,R11 / FR-004 / US3, SC-003/004 | T031–T039,T046,T047 | H4/reader/drop/framing/384 B; host.md та specs/03-rc-capture/verification/hardware.md |
| R12 / FR-005 / SC-003/004 | T001–T003,T007,T018,T029,T039–T042,T045–T051 | Environment, host, config, target build та physical розділені |
| R12 / FR-007 / SC-004 | T003,T042–T044,T049–T051 | Q03/H02/H03/прилад; verification/prerequisites.md, specs/03-rc-capture/verification/hardware.md |
| SC-002 / startup/LOW/fault clearing | T014,T016,T020–T029,T036,T044,T045,T047 | Effective0/permission false завжди; resync/reader не arming |
| D53 / CHK025-A | T008,T019,T020,T042,T045 | Точні width/18–22 мс boundaries і складні malformed — host; HJ 50 Гц physical обов’язкова, генератор не gate |
| D55 / C1-A | T052–T054,T038–T046 | Короткі accuracy windows128; all per-window pulses/intervals, frozen archive;600 с counts/timing окремо |
| D54 / CHK026-B | T042,T043,T045,T049–T051 | Для кожного correct pulse E_width≤5 мкс, кожного rising interval E_interval≤10 мкс; U_width/U_interval≤1 мкс окремо, без E+U запасу |

### Числові межі, які не можна втратити під час реалізації

- Valid width 900–2100 мкс; nominal 1000/1250/1400/1500/1800/2000→0/25/40/50/80/100%; period 18000–22000 мкс включно, unknown до двох rising. Validation перед clamp, без endpoint learning/filtering.
- RMT 1 МГц/APB, filter 0, stop5 мс,48 symbols/192 B, non-DMA/non-partial. Scratch копіюється перед rearm. Queues64 edges/8 batches, epoch/sequence; coherence та sanity≤50 мкс не є доказом accuracy5/10 мкс.
- Normal rearm≤1000 мкс від falling+5000; запас до наступного valid rising≥9900 мкс. Completion→core≤10000 мкс. Common stuck100000 мкс, fault deliveryдо 110000 мкс, без обіцянки строку текстового запису; activeguard 12000+10000 мкс, cleared on final.
- Capture timestamp estimated rising, end estimated falling; hardware RMT width незалежна. Last-valid тільки accepted valid end. Invalid activity, queued old sample і callback time не створюють freshness.
- Діагностика≤10records/с і ≥100000 мкс між attempts, один pending latest/один in-flight, max record 384 B/TX 512 B, zero-wait/drain gate/no history; overwrite/skip/write-drop counters різні та saturating.
- Physical accuracy у коротких D55 windows: direct E≤5/10 мкс, U≤1 мкс окремо. Невідоме/завеликеU — blocked, E понад допуск — fail. Host boundary proof не physicalpass.
- Zero valid loss за 600 с для кожного max diagnostics reader regime; actual analyzer count, не припущені 30000; rearm drops не виключаються. OutputLOW/permission false, насос/MD20A від’єднані.

## Стратегія реалізації

1. **MVP — тільки US1**: T001–T018, переносимі fixtures/normal capture та core integration із виходом 0. Це придатний для review input increment; без US2/US3 не оголошувати 03 готовою або фізично прийнятою.
2. Додати US2 з H2/H3, fault barrier і bounded recovery, не змінюючи core 02/arming.
3. Додати US3/H4, bounded transport та load instrumentation; виконати software checks/build без підключення плати.
4. Підготувати operator procedure та чесно лишити T043–T047 відкритими, доки окремо не дозволено і не виконано hardware work. Продовжити незалежні docs/evidence tasks.
5. Прийняти 03 лише через T051 після всіх застосовних доказів. Відкладений receiver перевіряється до використання, два входи —07, Q05/Q06 лишаються у власників.

## Підсумок генерації

54 задачі: Setup3, Foundational4, US1=11, US2=12, US3=20, Final4. Позначених [P] —17; це opportunities у наведених вікнах, не одночасність усіх 17. Кожна задача має path і R/FR/SC; story labels є лише у відповідних story phases. На момент генерації всі checkbox були unchecked; актуальний стан позначено вище.

Pre/post tasks hooks відсутні: .specify/extensions.yml не існує. Ця фаза створює лише tasks.md; Implement/Analyze/commit/push/flash/load автоматично не запускаються.

## Acceptance observability, analyze-guided C1-A

Accuracy proof: T052–T054 реалізують bounded collector/export/comparator, T038/T039 перевіряють їх на host, T041 — три target variants, T045 — фактичне коротке per-pulse comparison. T046 не є повним 600 с accuracy trace: він доводить тільки zero loss/counts і max latency під навантаженням. Production diagnostics/cutoff/arming policies не змінені, усі hardware tasks лишаються операторськими.

## Виконання Implement 2026-10-29

Користувач явно дозволив software/host/target builds із відкритими physical tasks. Виконано T001–T042,T048–T050,T052–T054 (48); T043–T047,T051 (6) відкриті. Host18/18 і tooling17/17; normal/stress/accuracy target builds/config checks pass. Це не iteration acceptance. Докази та R/FR/SC/constitution review — [acceptance](verification/acceptance.md). Hooks відсутні; flash/load/Git mutations не виконувались.

### Доповнення D56

T012/T027 уточнено прямим дорученням користувача: після new RMT RX встановлюється GPIO_FLOATING для GPIO3, з перевіркою помилки та cleanup. Host18/18,tooling17/17,три clean bias builds/config checks pass; compiled arguments/order/error branch перевірено у ELF. Докази — verification/acceptance.md (D56), попередні records збережено в verification/gpio-bias-before/. Фізичні tasks відкриті.

### Виправлення T035: USB RX capacity

RX64 виправлено на128 відповідно до strict SDK minimum >64; TX512/zero-wait/no commands незмінні. Host18/18,tooling17/17,три clean usb builds/config checks pass. Isolated RX64 regression експеримент падає як очікується. User boot log не є повним physical acceptance; hardware tasks лишаються відкритими.
