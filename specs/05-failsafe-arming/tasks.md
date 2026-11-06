# Задачі: прозора трансляція, Test/Fly та автоматичне відновлення05

**Вхід**: [spec.md](spec.md), [plan.md](plan.md), [research.md](research.md), [data-model.md](data-model.md), [контракт](contracts/operational-control.md), [quickstart.md](quickstart.md), [закритий checklist](checklists/operational-boundaries.md), [current-baseline](../../speckit-iteration/current-baseline.md).\
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming`.\
**Конституція**: v2.0.0, первинна ратифікація2026-10-28, поправка2026-11-06/D74; G05 закрито. D75 уточнює retry-епізод.\
**Дата**: 2026-11-06. **Стан**: Tasks сформовано; усі задачі нижче невиконані, це не звіт реалізації або прийняття.

## Правила виконання та шляхи

- Виконувати лише після окремого доручення Implement; ця фаза створює тільки `specs/05-failsafe-arming/tasks.md`. Не перемикати/створювати гілку, не commit/push, не відкривати пристрої, не flash/reset і не керувати навантаженням автоматично. D10 діє лише в Specify; попередню Git-публікацію checklist не переносити як дозвіл цієї фази.
- Під час кожного feature-scoped workflow явно задавати `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming` й перевіряти повернений шлях. `speckit-iteration/` — вхід, не місце генерації.
- Шляхи в задачах відносні до кореня репозиторію. Нові файли, явно названі нижче, є запланованими файлами всередині вже обраних Plan меж. Повторно використати native ESP-IDF v5.5.6/commit880a203ae603d08a94d06aa01e0d6761958e5e9e і `tools/environment/environment.lock.json`; core strict C++11, platform/app C++17, без Arduino/нового build system/SDK patch.
- Один ControlCore усередині CapturePolicy, один серіалізований rc-control/output owner. Public core без SDK/FreeRTOS/GPIO/I/O/heap. Немає другого control reducer, вихідної task або джерела команд. DIR постійно GND на MD20A; RC=GPIO3/PWM=GPIO0 лише погодженого профілю.
- Тести обов'язкові за FR-016 і конституцією; розташування test tasks перед implementation не вимагає test-first або окремого approval. Тестові файли й реєстрація CTest виконуються в межах відповідної історії, запуск після її реалізації. Host/build/physical докази записувати окремо, без вигаданих pass/counts.
- `[P]` означає лише незалежні файли в зазначеній нижче групі після її prerequisites, не паралельну зміну спільного core/output/owner. Цей документ описує можливості паралельної роботи, не запускає агентів.
- `[ОПЕРАТОР]` у тексті — окремий явно доручений фізичний крок. Не починати його за одним unchecked списком задач. Насос у всіх перевірках05 фізично від'єднаний; LOW на MD20A — brake, не ізоляція/coast/доведений нуль витрати. Engine start/combustion/recovery, насос06 та07–09 поза обсягом.

## Чинна основа й gates

01 має software/build докази й відкриті H02/H03/SC-004/SC-006; E01 закрито пізнішим записом у acceptance, не початковою таблицею. 02 виконав власні software критерії за D38. 03 має18 host/17 tooling/3 build результати, але T043–T047/T051 фізично відкриті. 04 має22 host/28 tooling/9 build/config результати,46/53 задач; T044–T050 фізично відкриті. Це історичні результати, не результати05. Прочитані acceptance01–04 та контракти02/03/04 лишаються regression/evidence boundary; старі артефакти не переписувати.

| Gate / власник | Стан і межа |
|---|---|
| G05 / governance | Закрито D74; перевірити чинний текст, не створювати повторну поправку |
| Q05 /05 | D69–D73, D75 і D76 обрані; задачі реалізують та перевіряють, не переобирають |
| Q02/H02/H03 /01 | Power/bias/startup/instrument/USB+external-power suitability до залежних фізичних дій; portable software не блокує |
| Q03 /03, джерело05 | Levels/conditioning/ground/timing до підключення/прийняття; actual ArduPilot/Matek version/revision/output/config і receiver/radio-loss до їх використання. HJ-only не залежить від відкладеного receiver gate й не приймає його |
| Q06 /04,06 | Actual MD20A revision/interface/DIR–GND до powered-driver tests; силове прийняття й насос тільки в06 після accepted05/перевіреного стенда |
| Q04 | Width900..2100/period18000..22000 inclusive, rounding/association/epoch/resync/stuck100+10мс вже обрані; фізичні результати pending |

## Фаза 1 — Підготовка та вихідні докази

**Мета**: зафіксувати дозволений software scope й відтворювану основу, не перезапускати розробку01–04.

- [ ] T001 Звірити D63–D75, шість gates конституціїv2.0.0, закритий G05 і actual acceptance01–04; зафіксувати початковий Git/source стан та owners відкритих physical gates у `specs/05-failsafe-arming/verification/acceptance.md` (FR-014,016,017/SC-008), без позначення залежних перевірок виконаними.
- [ ] T002 Перевірити pinned environment через `scripts/environment/check.py` і `tools/environment/environment.lock.json`; записати actual tools/SDK identity, команду/exit або blocker у `specs/05-failsafe-arming/verification/build.md` (успадкований R02/FR-016), не замінюючи версії й не використовуючи device I/O.
- [ ] T003 Після T002 отримати baseline host/tooling через `scripts/build.py`, `tests/host/CMakeLists.txt` і `tests/tooling/`; записати discovery/actual counts/exits, expected-failing negative_probe та source hashes у `specs/05-failsafe-arming/verification/host.md` (FR-004,009,016), зберігши старі build directories й докази01–04.

**Checkpoint**: вихідний стан й environment відомі; physical suitability не прийнята. За environment failure блокуються залежні запуски, а не вигадуються results.

## Фаза 2 — Спільні контракти та супервізія

**Мета**: підготувати існуючі core/adapter boundaries для всіх історій; ці задачі — prerequisites для story implementation.

- [ ] T004 [P] Розширити `firmware/components/bridge_core/include/bridge/control.hpp` та `firmware/components/bridge_core/include/bridge/rc_capture.hpp`: immutable LegacyDisabled/Test/Fly, AcceptedMeasurement, окремі accepted-end/service-now ordering, ControlSnapshot05/validity/generation/epoch та owner-only entrypoints за контрактом§1; default constructor/API02 залишити legacy (FR-001,003,005,008/SC-001,002).
- [ ] T005 [P] Визначити roles LegacyDisabled/FiniteBench04/Operational05, stage/route/OutputHealth, checked request/result і recovery episode DTO у `firmware/components/bridge_core/include/bridge/pump_output.hpp` та `firmware/components/platform_esp32/include/platform/pump_output.hpp`; не змішувати readiness, build family й core mode; чинні finite04 types/semantics зберегти (FR-010/SC-005, D75).
- [ ] T006 Створити `firmware/components/platform_esp32/include/platform/control_watchdog.hpp` і `firmware/components/platform_esp32/control_watchdog.cpp`: testable supervision port та ESP_PLATFORM leaf init/add/reset/emergency-reset з перевіреними результатами; використати checked ClockReading boundary з `firmware/components/platform_esp32/include/platform/startup_port.hpp`, без feed з іншої task/ISR (FR-011,012/SC-005).
- [ ] T007 Створити окремий `firmware/components/platform_esp32/control_watchdog_reset.cpp`: strong extern-C IRAM hook без weak prototype esp_task_wdt.h, прямий ROM system reset/no-return fallback без GPIO/USB/heap/RTOS/logging; включити обидва watchdog TU тільки operational через `firmware/components/platform_esp32/CMakeLists.txt` з pinned esp_system/esp_rom dependencies (FR-012/SC-005, D71).
- [ ] T008 Додати shared fake clock/SDK/supervision fixtures у `tests/host/operational_fixtures.hpp` і host-testable seams того самого owner/adapter через `tests/host/CMakeLists.txt`; SDK/RTOS leaves лишити під ESP_PLATFORM, розширити `tests/host/control_header_probe.cpp` для strict C++11/new DTO/RAM≤2048B без hardware headers, не створюючи паралельної моделі керування (FR-001,010,012,016/SC-001,008).

**Checkpoint**: T004–T008 завершені перед T009–T054; runtime supervision API існує, але тільки історії нижче реалізують повний qualified progress/failure contract. Фізичних запусків частково інтегрованого образу немає.

## Фаза 3 — US1: прозора команда після startup/reset (P1), перший програмний зріз

**Мета**: LOW без current-boot valid, перша fully-valid high без zero/arming/extra10frames; ordinary/default build Test, Fly лише явно.

**Незалежний критерій**: fake clock/SDK перевіряють US1.AC1–4, шість точок і reset-reconstruction; clean configure доводить defaultTest/explicitTest/Fly. Physical P1/P2 виконуються лише T061/T062 після фінальної readiness, не за результатом цієї фази.

### Перевірки US1

- [ ] T009 [P] [US1] Додати `tests/host/operational_startup_tests.cpp`: Test/Fly no-valid/invalid/startupHIGH/unknown period→LOW, first fully-valid2000→100% без zero/arm/extra frames, valid0 з permissiontrue, шість nominal points/rounding/invalid hold та новий boot без старого duty; зареєструвати suite через T017 (FR-003,004/SC-001,002).
- [ ] T010 [P] [US1] Додати `tests/host/operational_capture_tests.cpp`: справжній CapturePolicy/owner path не commit-ить candidate до processing_complete/barrier check, не підміняє end часом fault-now, зберігає legacy API і один core; fake output transcript охоплює LOW↔PWM↔HIGH (FR-001,005,009,010/SC-001,002).
- [ ] T011 [P] [US1] Додати `tests/tooling/test_operational_build_contract.py`: ordinary/explicitTest/Fly/legacy/finite04 composition, direct native default, manifest identity і негативні host+target/unknown-mode/operational+legacy/03/04 combinations без виклику hardware (FR-002/SC-002).

### Реалізація US1

- [ ] T012 [US1] Реалізувати owner-only accepted valid/startup/snapshot path у `firmware/components/bridge_core/control.cpp`, перевикориставши validation-before-clamp/map02; technical/no-valid priority, calculated/effective/applied/permission/age validity не змішувати, legacy clear/update semantics не змінювати (FR-003–005,008,010/SC-001,002).
- [ ] T013 [US1] Передати config через існуючий CapturePolicy і реалізувати provisional parse→processing check→commit у `firmware/components/bridge_core/rc_capture.cpp`; у `firmware/components/platform_esp32/rc_capture.cpp` та `firmware/components/platform_esp32/include/platform/rc_capture.hpp` відокремити persistent owner/notify timer від ingress resources й зберегти shared-lock/epoch/barrier protocol03. Тут же виконати спільну міграцію Operational05: current input status/sticky history відокремлені від technical inhibition, signal fault не викликає legacy update_capture_fault(now), observer не OR-ить sticky capture bits у terminalLOW. Зберегти discard/epoch/barrier/resync/stuck100+10мс і незмінні legacy02/03/finite04 semantics; ця міграція потрібна обом режимам до checkpoint US2 (FR-005,008,009,010/SC-001,002,004).
- [ ] T014 [US1] Розширити `firmware/components/platform_esp32/pump_output.cpp` для Operational05: stage-aware LEDC initialization з duty0 і checked set0/update/stop0, дозволені requests/generation/epoch, staticLOW/HIGH та count=(p×8192+50)/100 для1..99%; не використовувати bench_ як operational permission і не змінювати finite04 path (FR-004,010/SC-002,007).
- [ ] T015 [US1] Реалізувати immutable build composition у `firmware/main/Kconfig.projbuild`, `firmware/sdkconfig.defaults`, нових `firmware/sdkconfig.operational05.defaults`/`firmware/sdkconfig.legacy05.defaults`, `scripts/build.py` та `scripts/verify/config.py`: ordinary/direct native Test, explicit `--operational-mode test|fly`, `--legacy-disabled`, legacy03/finite04 fragments, full manifest hashes/policy constants і rejection змішаних flags (FR-002/SC-002, D72).
- [ ] T016 [US1] Інтегрувати profile→primedLOW→foundation5attempts/1с→watchdog/owner/timer readiness→single handoff→outputReady→нове capture спостереження у `firmware/main/app_main.cpp`, `firmware/components/platform_esp32/startup_policy.cpp` та `firmware/components/platform_esp32/rc_capture.cpp`; до init/add/handoff success nonzero заборонений, після handoff app не пише вихід; startup failure history не permanent lock operational (FR-003,010,012/SC-002).
- [ ] T017 [US1] Зареєструвати T009/T010 у `tests/host/CMakeLists.txt`, запустити story suites/legacy regression/tooling T011 і preliminary Test/Fly configure/build без плати; записати команди/actual results та ще непокриті US2–US5 у `specs/05-failsafe-arming/verification/host.md` і `specs/05-failsafe-arming/verification/build.md` (FR-016/SC-001,002,008).

**Checkpoint/MVP**: програмний startup/valid-command зріз. Це не повне прийняття US1 або дозвіл фізичного operational output; Test/Fly loss, output recovery, qualified feed і diagnostics завершуються наступними історіями.

## Фаза 4 — US2: Test cutoff і автоматичне повернення (P1)

**Мета**: age≥500000мкс вимагає LOW; missing/invalid/stuck не переносять t_v; нова trustworthy high відновлює вихід.

**Незалежний критерій**: US2.AC1–5 у керованому clock/SDK, healthy owner lateness≤1мс, deadline→checkedLOW≤2мс, accepted end→checked apply≤19мс. Фізичний last-valid→LOW≤1с — окремий T063, synthetic host не доводить його.

### Перевірки US2

- [ ] T018 [P] [US2] Додати `tests/host/operational_timeout_tests.cpp`: age499999/500000/500001мкс, no-prior-valid, repeated same-time poll, missing/invalid-width/period/incomplete/stuck/overflow без refresh, cutoff→new-high/valid0 без manual clear; перевірити збережений calculated при effective0 (FR-005,006,008/SC-001,003).
- [ ] T019 [P] [US2] Додати `tests/host/operational_deadline_tests.cpp`: фактичний owner/SDK path з late worker/output, pending fault перед/між set/update, deadline і new-valid race, expired/pre-barrier/duplicate samples, stale generation після LOW; lateness записується фактично, не backdated (FR-005,006,010/SC-003).

### Реалізація US2

- [ ] T020 [US2] Реалізувати Test hold/timeout/recovery у `firmware/components/bridge_core/control.cpp`: checked now−accepted pulse_end_us, T=500000 inclusive, без переносу deadline invalid/fault/status/worker/retry подіями, без arming/zero handshake (FR-005,006,008/SC-003, D69).
- [ ] T021 [US2] Додати evaluate з fresh now до/після кожного nonzero SDK call, включно зі stopIdle1 і між set/update, у `firmware/components/platform_esp32/rc_capture.cpp` та `firmware/components/platform_esp32/pump_output.cpp`; pending ingress під коротким lock, SDK поза lock, stale request не скасовує LOW; забезпечити notify1мс/healthy budgets без послаблення03 (FR-006,009,010/SC-003).
- [ ] T022 [US2] Додати explicit bench-only timing journal/token→API brackets у `firmware/components/bridge_core/include/bridge/pump_output.hpp`, `firmware/components/bridge_core/pump_output.cpp` та `firmware/components/platform_esp32/pump_output.cpp`:≤64×48B+header128B за D76: до коду collector зафіксувати в `specs/05-failsafe-arming/contracts/operational-control.md` bounded layouts/stable codes/canonical digest, compile-time scenario/start/final event та record-count bound з anchors/API results. RAM accepted anchor оновлюється кожною accepted valid; record лише для transitions/пов’язаних API brackets/all failures/late/retry/D75 closure, не repeated success. Freeze після final checked result; missing anchor/result/overflow→incomplete, без production history або cutoff Fly через recorder full; export інтегрується T050 (FR-015/SC-003,008, D73).
- [ ] T023 [US2] Розширити `tests/host/operational_deadline_tests.cpp` перевірками accepted-end→API≤19мс, cutoff lateness≤1мс/LOW≤2мс, first-failure aggregateLOW≤1мс і faults у timing recorder; для healthy input-loss P3 LOW-рішення при age<T відхиляється як premature cutoff, а technical LOW має окрему причину/сценарій; не підставляти API result як physical voltage/1с proof (FR-006,010,015/SC-003,005).
- [ ] T024 [US2] Зареєструвати T018/T019 у `tests/host/CMakeLists.txt`, запустити US2+US1/legacy suites й записати actual results у `specs/05-failsafe-arming/verification/host.md`; у `specs/05-failsafe-arming/quickstart.md` узгодити реальні команди й future P3 з незмінним direct physical1с критерієм (FR-006,015,016/SC-003,008).

**Checkpoint**: модель і checked API Test поведінка перевірені; physical1с ще not_run.

## Фаза 5 — US3: тривалий Fly hold та recovery після resync (P1)

**Мета**: справний Fly утримує0/intermediate/100% за input loss; жодний старий latch/observer/terminal04 не обходить це правило.

**Незалежний критерій**: US3.AC1–4 з великим age й capture-loss fixtures, нові high/low лише після двох trustworthy rising та fully-valid commit. Physical0/80/100% hold≥600с при reader active/slow/absent — T064.

### Перевірки US3

- [ ] T025 [P] [US3] Додати `tests/host/operational_fly_tests.cpp`: hold0/25/80/100 довше100+10мс/T/1с і age>UINT32_MAX, missing/invalid/stuck без cutoff, no-valid→LOW, reset discards duty, resync сам не команда, наступна new-high/new-low автоматичні (FR-003,007,008/SC-002,004).
- [ ] T026 [P] [US3] Додати `tests/host/operational_resync_tests.cpp`: overflow/drop/48-symbol saturation/ambiguous association/pre-barrier/old/duplicate rejection, two-rising resync, current-vs-history, ended stuck episode/masked interval, RMT/GPIO failure і D56 floating-pulls cleanup; actual owner/observer не terminal у Fly, legacy04 лишається terminal (FR-005,007–009/SC-001,004).

### Реалізація US3

- [ ] T027 [US3] Реалізувати Fly policy у `firmware/components/bridge_core/control.cpp`: input absence/history не інвалідовують held command/permission/generation, no-prior-validLOW та технічний пріоритет залишаються; не додавати Fly freshness cutoff чи periodic reboot (FR-007,008/SC-004).
- [ ] T028 [US3] Інтегрувати Fly policy T027 зі спільним current input/history/technical та observer path, уже мігрованим у T013, у `firmware/components/bridge_core/rc_capture.cpp` і `firmware/components/platform_esp32/rc_capture.cpp`; перевірити hold при input fault, sticky history і new-valid recovery після resync, без нового blanket inhibition; legacy02/03/finite04 branch незмінний (FR-005,007–009/SC-001,004).
- [ ] T029 [US3] Завершити ingress retry≥100мс/cleanup за known resources у `firmware/components/platform_esp32/rc_capture.cpp`: worker/notify timer/output ownership переживають RMT/GPIO teardown, monitor обліковує observation gaps, новий RX повторно задає GPIO_FLOATING; output/Test deadline/watchdog owner не зупиняються, ресурси/черги64/8/48 не ростуть (FR-007,009,010/SC-004,006).
- [ ] T030 [US3] Додати `tests/host/operational_time_tests.cpp`: окреме accepted/service ordering, uint64 age/checked arithmetic, modulo32 seq/epoch bounded span<2^31, bounded tick wrap, generation overflow fatal, statistics saturation без Fly cutoff і association-loss saturation з resync (FR-005,007,011/SC-001,004,005).
- [ ] T031 [US3] Зареєструвати T025/T026/T030 у `tests/host/CMakeLists.txt`, запустити US1–US3/legacy regression й Test/Fly target compile без пристрою; записати `specs/05-failsafe-arming/verification/host.md` та `specs/05-failsafe-arming/verification/build.md`, залишивши long physical hold pending (FR-007,016/SC-004,008).

**Checkpoint**: input-loss policy обох режимів і capture migration перевірені software; Fly не оголошується стійким до failed output/reset.

## Фаза 6 — US4: технічні відмови, retry D75 та watchdog (P1)

**Мета**: bounded technical recovery без stale replay, honest LowUnconfirmed та reset stalled actual owner; input-loss hold не підміняється технічною відмовою.

**Незалежний критерій**: US4.AC1–5 та SC-005 через fake SDK/clock/supervision; кожний checked API/stage,3 додаткові reinit starts/≥100мс/≤20мс attempt, D75 episode closure лише post-fence valid apply. Target audit доводить config/strong IRAM hook, але reset350мс/LOW100мс і physical Test1с доводять лише T065/T066.

### Перевірки US4

- [ ] T032 [P] [US4] Додати `tests/host/operational_output_failure_tests.cpp`: failures кожного init/apply/stop/fallback/pause/deconfigure/reinit stage, route ownership після GPIO fallback, aggregate fault→LOW≤1мс без rebase, per-call1мс/attempt20мс misses, LowUnconfirmed і idempotent/stale/wrong-generation requests (FR-010/SC-005).
- [ ] T033 [P] [US4] Додати `tests/host/operational_retry_tests.cpp`: перша операція не retry, кожний reinit start витрачає одну з3; reinit0-success/apply-failure не скидає count; третій success→new post-fence valid0/high success закриває епізод, failure до цього→ExhaustedLow; наступний незалежний епізод має3, history збережена, serviceLOW/valid commit не closure (FR-010/SC-005, D75).
- [ ] T034 [P] [US4] Додати `tests/host/control_watchdog_tests.cpp`: init/add failure→SetupBlockedLOW/nohandoff без reset claim, post-handoff feed error→emergency reset, qualified complete checkpoint лише owner, Test nonzero feed тількиage<T/післяT лишеLOW, Fly no-RC/no-reader й ExhaustedLow progress не stall; notification/diagnostic/dequeue/frozen raw не feed (FR-012/SC-005).
- [ ] T035 [P] [US4] Додати `tests/host/operational_clock_tests.cpp`: unavailable/negative/regressive/frozen raw, arithmetic overflow та failure між SDK calls/ISR leaves; old sample за справного clock→discard/resync, fatal shared clock/protocol/LowUnconfirmed після armed subscription→best-effortLOW/ResetPending/no feed (FR-011,012/SC-005).

### Реалізація US4

- [ ] T036 [US4] Реалізувати checked failure/fallback/known-stage cleanup/staged reinit FSM у `firmware/components/platform_esp32/pump_output.cpp` та `firmware/components/bridge_core/pump_output.cpp`: stop0 або обидва GPIO preload0/direction calls, відновлення LEDC route тількиduty0, окремі checked set/update/stop після channelconfig, spacing≥100000мкс/per-call≤1мс/attempt≤20мс з ingress interleaving, без repeated LEDC context allocation (FR-010/SC-005).
- [ ] T037 [US4] Реалізувати recovery epoch/fence й D75 episode counters у `firmware/components/platform_esp32/pump_output.cpp`, `firmware/components/bridge_core/control.cpp` та `firmware/components/platform_esp32/rc_capture.cpp`: first failure і last failure completion різні, reinit0 не closure, лише checked новий valid end>recovered_at/apply актуальних epoch/generation (включно з0%) поновлює бюджет; exhausted confirmedLOW до reboot без app auto-restart (FR-010/SC-005).
- [ ] T038 [US4] Замінити unchecked raw-time casts у `firmware/components/platform_esp32/rc_capture.cpp`, `firmware/components/platform_esp32/pump_output.cpp` та `firmware/components/platform_esp32/startup_policy.cpp` на checked signed64/current-order/overflow boundary для owner/ISR/SDK leaves; invalid age лишити invalid, не синтезувати now=0 або freshness із callback (FR-005,011/SC-001,005).
- [ ] T039 [US4] Завершити actual-progress/feed/failure routing у `firmware/components/platform_esp32/control_watchdog.cpp`, `firmware/components/platform_esp32/rc_capture.cpp` та `firmware/main/app_main.cpp`: runtimeTWDT100мс тількиowner/idle mask0/auto-init off, IWDT500мс, до supervision readinessLOW; fatal/LowUnconfirmed no-feed, feed API error→best-effortLOW/явний ROM reset, no direct MWDT reprogram/no new writer (FR-012/SC-005).
- [ ] T040 [US4] Додати compile-time one-shot timing/stall fault fixtures до `firmware/components/platform_esp32/rc_capture.cpp` та `firmware/components/platform_esp32/control_watchdog.cpp`: після відомої accepted/apply події owner block/busyloop/IRQ-cache stall і поодинокі output/clock injections; production без injection, нових RC/USB commands/GPIO marker/NVS breadcrumb/retained duty немає (FR-012,015/SC-005, D73).
- [ ] T041 [US4] Розширити `scripts/verify/config.py` і `tests/tooling/test_operational_build_contract.py` для actual TWDT100мс/runtime only-owner/IWDT500мс, strong retained hook/ROM target/IRAM/no-return, LOGmax/default0 без SDK pre-hook output, debugger/GDB/coredump/apptrace off; legacy03/04 не отримують hook або operational settings (FR-012,016/SC-005).
- [ ] T042 [US4] Зареєструвати T032–T035 у `tests/host/CMakeLists.txt`, виконати actual adapter/core/watchdog failure matrix та попередні історії; зафіксувати transcripts/clock boundaries/D75 results/negative cases у `specs/05-failsafe-arming/verification/host.md` (FR-010–012,016/SC-005).
- [ ] T043 [US4] Зібрати clean Test/Fly без hardware й виконати nm/map/disassembly/IRAM/ROM closure audit, actual subscription/feed callsite review та RAM≤2048B; записати `specs/05-failsafe-arming/verification/build.md` і `specs/05-failsafe-arming/verification/config-review.md`, відокремивши conditional950мс allocation від невиміряних350/100мс/physical1с (FR-012,015,016/SC-005,008).

**Checkpoint**: software recovery/supervision перевірені; standard panic10с не зараховано як cutoff guarantee. Watchdog — не незалежна ізоляція й не доказ довільної hardware fault tolerance.

## Фаза 7 — US5: актуальна bounded діагностика й evidence (P2)

**Мета**: спостережувані mode/input/calculated/effective/applied/current/history без впливу USB на керування; bench evidence окремо від production.

**Незалежний критерій**: US5.AC1–4 через formatter/slot/transport/collector/parser fixtures,≤384B/≥100мс/≤10записів/с, one-committed reconnect caveat. Actual max-load capture/output та stack high-water — T067/T068, не host pass.

### Перевірки US5

- [ ] T044 [P] [US5] Додати `tests/host/operational_diagnostics_tests.cpp`: rc05v1 fixed grammar/codes/masks/validity, Test calculated80/effective0 проти Fly history+effective80, valid0 проти no-valid, SDK status/op/retry/progress/reset-reason, максимальні дозволені значення/overflow/refusal/no truncation та conservative≤347B/384B padded (FR-013/SC-006).
- [ ] T045 [P] [US5] Додати `tests/host/operational_transport_tests.cpp`: overwrite/concurrent publish/version ack, busy/zero/partial writes,≥100мс/one committed/latest після reconnect, archive замість snapshot в одному слоті, transport never feed/control/output, progress за відсутності читача (FR-013/SC-006).
- [ ] T046 [P] [US5] Додати `tests/host/operational_evidence_tests.cpp`: journal64×48+128/accuracy128×48+256/stress≤512B, half-open bounds start inclusive/end exclusive за accepted end, належні samples з processing після end, watermark/drain і closure≤17мс, unknown provenance/overflow/frozen/postfreeze-invalid/mixed identity, RAM reset loss; D76 cases: довгий незмінний потік не заповнює journal, останній accepted anchor фіксується перед подією, all failures/late/retry та valid0 D75 closure збережені, compile-time start/final checked freeze правильні, record bound включає anchors/API, missing anchor/result→incomplete; freeze/full не зупиняє production-like Fly/Test service, accuracy stop явно bench-only (FR-015/SC-006,008).
- [ ] T047 [P] [US5] Додати `tests/tooling/test_operational_trace.py`: synthetic parser/comparator fixtures для H/R/E identity/count/digest, unknown/missing enum/field, mixed/partial/overflow archive, accepted-source matching/gaps/static endpoints, latency999999/1000000/1000001мкс з U, new-valid20мс та600с lower-bound; D76 missing/mismatched accepted anchor, відсутній final checked result або прихована failure/D75 closure не дають valid timing archive; окремі premature-LOW/hold-before-T fixtures, у тому числі110мс cutoff при T500мс, і unknown clock-correlation→blocked замість pass за самою верхньою межею1с; host fixtures не physical evidence (FR-015/SC-003–008).

### Реалізація US5

- [ ] T048 [US5] Розширити coherent snapshot та rc05 formatter у `firmware/components/platform_esp32/include/platform/startup_port.hpp` і `firmware/components/platform_esp32/startup_policy.cpp`: model fields/validity/current-history/applied/retry/progress, frame capacity385 з NUL/≤384B transport; відокремити RAM snapshot sizing від frame limit, незмінні rc04/rc03-trace grammars (FR-013/SC-006).
- [ ] T049 [US5] Інтегрувати rc05 та bench archive choice у `firmware/components/platform_esp32/local_diagnostics.cpp` і `firmware/components/platform_esp32/include/platform/local_diagnostics.hpp`: latest-only, sole writer, TX512/RX128/drain gate/zero-wait/drop/spacing≥100мс, formatting поза lock, максимум один committed record; reconnect не очищає history/не створює команду (FR-013/SC-006).
- [ ] T050 [US5] Завершити bench timing/stall/accuracy/stress collectors/export у `firmware/components/bridge_core/pump_output.cpp`, `firmware/components/bridge_core/rc_capture_trace.cpp` і `firmware/components/platform_esp32/pump_output.cpp`; використати bounded layouts/stable codes/canonical digest serialization, уже зафіксовані T022 у `specs/05-failsafe-arming/contracts/operational-control.md`, для одного D76 event collector/formatter/parser без повторного вибору політики. Зберегти model RAM limits/immutable H/R/E/live fault flags, stress[t0+1с,t0+602с) за accepted end, producer watermark/epoch closure і drain усіх належних samples до≤17мс без output cutoff; сам timer не доказ closure. Accuracy лише checkedLOW→stop→export; overflow робить evidence incomplete (FR-015/SC-006, D73).
- [ ] T051 [US5] Реалізувати тільки file-based `scripts/verify/operational_trace.py`: reject malformed/mixed/incomplete archives, повний manifest/boot/source provenance, одна analyzer timebase/known initial levels/gapless bounds, L+U≤latency limit при U≤1мс, незалежно доведене hold-before-T за healthy P3 (API age і physical state окремо, unknown correlation/uncertainty→blocked, достовірно earlyLOW→fail), independent reset anchor й D54/M04 E/U без averaging/offset fitting; missing method→blocked, established violation→fail, жодного serial/flash/reset side effect (FR-015/SC-003–008).
- [ ] T052 [US5] Додати explicit `--pump05-evidence timing|stall|accuracy|stress` composition у `scripts/build.py`, `firmware/main/Kconfig.projbuild` і новий `firmware/sdkconfig.pump05-evidence.defaults`; purpose/mode/source/config hashes у manifest, normal Test/Fly без archives/injection, rejection із03/04/legacy flags, extra bench RAM≤12288B й незмінні finite04 images (FR-002,013,015/SC-002,006,008).
- [ ] T053 [US5] Зареєструвати T044–T046 у `tests/host/CMakeLists.txt`, виконати host/tooling T047 і actual formatter/parser/collector interoperability з negative cases та interleaved control/USB congestion; записати команди/exits/RAM sizing/rc05 byte lengths у `specs/05-failsafe-arming/verification/host.md` (FR-013,015,016/SC-006,008).
- [ ] T054 [US5] Узгодити реальні build/verifier commands і evidence inputs у `specs/05-failsafe-arming/quickstart.md`, залишивши окремі host/build/operator секції й one-committed reconnect caveat; записати US5 results/limitations у `specs/05-failsafe-arming/verification/acceptance.md`, не закриваючи physical gates за synthetic tests (FR-014–017/SC-008).

**Checkpoint**: завершено software increment усіх п'яти історій. Перш ніж operator дії, потрібні fresh regression/build/config review та конкретна readiness нижче.

## Фаза 8 — Спільна регресія, окреме фізичне прийняття та звіт

### Host і target без плати

- [ ] T055 Виконати fresh повний host/tooling regression у `tests/host/CMakeLists.txt` та `tests/tooling/` після останньої зміни, включно з legacy02/03/04 suites, header isolation і expected-failing negative_probe; actual discovery/counts/commands/exits/source hashes записати у `specs/05-failsafe-arming/verification/host.md` (FR-001,004,009,016/SC-001–006,008).
- [ ] T056 Через `scripts/build.py` виконати clean target matrix ordinary-default/explicitTest/Fly/LegacyDisabled,03 diagnostics/accuracy,04 waveform/stress/accuracy0..5 та05 timing/stall/accuracy/stress у кожному Test/Fly; direct native clean configure без modeflag окремо доводить Test. Записати effective config/manifests/artifact hashes/exits у `specs/05-failsafe-arming/verification/build.md`; старі build directories не видаляти (FR-002,016/SC-002,008).
- [ ] T057 Виконати actual config/compile graph/nm/map/disassembly/IRAM/ROM review через `scripts/verify/config.py`: radios/network/Arduino/DIR/fade відсутні, C++11/17 pinned, one owner/writer/qualified feed, legacy hook absent, production archive absent, RAM≤2048B/bench≤12288B; зафіксувати limits й ще невиміряний runtime stack у `specs/05-failsafe-arming/verification/config-review.md` (FR-001,012,013,015,016/SC-005,006,008).

### Operator readiness і фізичні кроки — тільки за окремим дорученням

**Передумови T059–T068**: T055–T057 успішні для exact binaries, окреме явне operator завдання, насос від'єднаний. T058 готує документ, не відкриває hardware. За невідомих gates наступні залежні задачі лишаються unchecked/blocked. Перевірки01/03/04 можна посилати як чинний доказ лише коли він реально є та застосовний до exact setup; відсутні результати не вигадувати.

- [ ] T058 Підготувати в `specs/05-failsafe-arming/verification/hardware.md` відтворювану процедуру P1–P7 із exact image/mode/boot/source, instrument/U/calibration/probe loading, wiring/power/initial levels, gapless acquisition/raw provenance, source matching/reset anchor/repetitions і predeclared M05/M04/D53–D55 limits; для P3 до run задати незалежне зіставлення MCU age/accepted anchor з physical waveform, clock/timebase uncertainty та класифікацію premature-LOW/неоднозначного boundary без guessed output offset; невідомі physical inputs явно blocked, без flash/device commands (FR-014,015/SC-003–008).
- [ ] T059 [ОПЕРАТОР] Прийняти застосовні Q02/H02/H03/Q03/Q06 readiness: profile/unit/3V3/bias/USB power compatibility, actual input conditioning/levels/ground/protection, MD20A revision/interface/DIR–GND без насоса та придатний прилад/U; actual evidence або blockers записати у `specs/05-failsafe-arming/verification/hardware.md` із посиланнями на acceptance01/03/04 (FR-014/SC-002,007,008).
- [ ] T060 Для обраного джерела записати в `specs/05-failsafe-arming/verification/hardware.md` actual HJ-only scope або, перед використанням Matek/ArduPilot/receiver, [ОПЕРАТОР] перевірити його version/revision/output/PWM configuration/електричні рівні та concrete radio-loss/failsafe behavior; за HJ-only залишити receiver gate deferred-before-use, не робити його умовою HJ acceptance і не заявляти radio-health detection за valid PWM (FR-014/SC-008, D48/D63).
- [ ] T061 [ОПЕРАТОР] Виконати P1 для Test/Fly на GPIO0 і вході accepted MD20A: no current-boot valid→LOW, first fully-valid high, reset із80/100% без prior-duty restore; cold/manual/software/watchdog/brownout/bootloader/upload, pretrigger≥10мс/≥5 повторів на тип/без unwantedHIGH≥1мкс до valid. Записати raw/analog/provenance/gaps/expected-actual у `specs/05-failsafe-arming/verification/hardware.md` (US1/FR-003,014,015/SC-002,007).
- [ ] T062 [ОПЕРАТОР] Виконати P2 у Test/Fly: шість nominal points, LOW/HIGH static endpoints та high↔PWM↔LOW, per-cycle1000Гц/M04 E_f≤1Гц/U_f≤0,1Гц і duty E≤0,1п.п./U≤0,02п.п., LOW+U≤0,8В/HIGH−U≥2,4В/HIGH+U≤3,6В/U_V≤0,05В; new-valid→physical state≤20мс за L+U. Докази у `specs/05-failsafe-arming/verification/hardware.md` (US1/FR-004,015/SC-001,007).
- [ ] T063 [ОПЕРАТОР] Виконати P3 Test від80/100%: missing/invalid-width/period/incomplete/stuckHIGH/LOW/capture loss окремо, accepted token однозначно зіставлений із physical GPIO3fall; утримання80/100% при age<T за справних clock/output та L+U≤1с/U≤1мс до початку continuousLOW≥100мс без нової valid, далі reconnect-high≤20мс без zero/manual clear. Достовірний prematureLOW→fail; upper-bound1с сам не pass. Якщо незалежний метод зіставлення age/physical state з uncertainty не відрізняє early cutoff від допустимої межі — hold-part blocked, combined P3 не pass; не додавати довільну physical lower-bound500мс/новий допуск. API brackets/healthy budgets окремо, фізичний результат у `specs/05-failsafe-arming/verification/hardware.md` (US2/FR-005,006,008,015/SC-003,007).
- [ ] T064 [ОПЕРАТОР] Виконати P4 Fly input-loss hold0/80/100%≥600 фізичних секунд для active/slow/absent reader й заявлених missing/invalid/stuck/resync cases, gapless static endpoints/без hidden cutoff; після належного resync нова high/low команда≤20мс, reset discard окремо. Записати duration lower bound/U/initial levels/source match у `specs/05-failsafe-arming/verification/hardware.md` (US3/FR-007,008,015/SC-004,007).
- [ ] T065 [ОПЕРАТОР] Виконати P6 тільки exact bench05 images по одній погодженій output/clock injection: checked/fallback/route/LowUnconfirmed, bounded3/100мс retry і D75 reinit-success/apply-failure/valid0 closure/exhaustion; MCU aggregateLOW≤1мс/per-call≤1мс/attempt≤20мс окремо від physical voltage, raw invalid→видимий reset path. Evidence й limits coverage записати у `specs/05-failsafe-arming/verification/hardware.md` (US4/FR-010,011,015/SC-005).
- [ ] T066 [ОПЕРАТОР] Виконати P7 Test/Fly без debugger/fresh boot: one-shot owner block/busyloop/IRQ-cache stall при відомій accepted/apply події, diagnostics alive не feed, no-RC/no-reader Fly control runs не reset; незалежний reset anchor доводить feed→reset≤350мс і reset→physicalLOW≤100мс, direct Test last-valid→LOW≤1с окремо. Pre-reset RAM evidence зберегти зовні; missing anchor/невідоме U→blocked partialmetric, ROM software reason не watchdog proof. Запис у `specs/05-failsafe-arming/verification/hardware.md` (US4/FR-012,015/SC-002,005).
- [ ] T067 [ОПЕРАТОР] Виконати P5 short accuracy у Test/Fly: усі128 records без відбору, per-pulse E_width≤5мкс/per-rising E_interval≤10мкс, U≤1мкс окремо за D54, source/epoch/seq matching, stuck100+10мс/rearm≤1мс/processing≤10мс/guard12+10мс; bench checkedLOW→stop→export не видавати за Fly hold. Raw/archives/comparator results записати у `specs/05-failsafe-arming/verification/hardware.md` (US5/FR-009,013,015/SC-001,006).
- [ ] T068 [ОПЕРАТОР] Виконати P5 healthy50Гц stress у Test/Fly з max384B/100мс, active/slow/absent reader/reconnect: окремі gapless≥600 physicalс zero-loss/timing, stress601MCUс bounds/closure≤17мс та D62 first/last-matched-fall lower bound без nominal30000 припущення; freeze не cutoff Fly. Повторити застосовні US1–US4 transitions під max diagnostics, перевірити bytes/drop/reconnect/owner timing/RAM/stack high-water; results у `specs/05-failsafe-arming/verification/hardware.md` (US5/FR-009,013,015/SC-003–006,008).

### Простежуваність і чесне завершення

- [ ] T069 Звести actual R/FR/US/SC→T→host/build/config/physical evidence з командами/config/source/artifact hashes/expected-actual/status у `specs/05-failsafe-arming/verification/acceptance.md`; якщо T059–T068 blocked, оформити software-only проміжний звіт і зберегти їх unchecked, не оголошувати full05/готовність06 або всю T069 виконаною (FR-016,017/SC-008).
- [ ] T070 Звірити `specs/05-failsafe-arming/tasks.md`, `specs/05-failsafe-arming/quickstart.md` та `specs/05-failsafe-arming/verification/acceptance.md` з actual evidence й усіма шістьма gates; уточнити поточний статус у `docs/roadmap.md`/`docs/open-questions.md` лише за доказами, не переписуючи ADR/history01–04, не запускаючи наступну фазу/насос06/commit/push (FR-014,016,017/SC-008).

## Залежності та порядок завершення

```text
T001 -> T002 -> T003
                 |
     {T004,T005} -> T006 -> T007 -> T008
                                         |
                              US1 T009–T017
                                         |
                              US2 T018–T024
                                         |
                              US3 T025–T031
                                         |
                              US4 T032–T043
                                         |
                              US5 T044–T054
                                         |
                        T055 -> T056 -> T057 -> T058
                                         |
                       explicit operator task + T059/T060
                                         |
                    T061 -> T062 -> T063 -> T064 -> T065
                                         |
                              T066 -> T067 -> T068
                                         |
                                    T069 -> T070
```

- T004/T005 мають спільний завершений prerequisite T003 й різні файли. T006 використовує погоджені DTO; T007 після T006; T008 реєструє actual host seams/probes. US1 після T008, подальша інтеграція серіалізована через спільні control.cpp/rc_capture.cpp/pump_output.cpp, не через нову архітектуру.
- У кожній історії незалежні test-file tasks можна писати паралельно за вже визначеними контрактами; реалізація й CMake registration мають одного послідовного редактора. Test suites запускаються після відповідних implementation tasks; завершення story code не означає фізичного прийняття.
- T017 залежить від T009–T016; T024 від T018–T023 та завершеної спільної міграції T013 (через T017); T028 більше не є відкладеною передумовою Test hold/recovery; T031 від T025–T030; T042 від T032–T041, T043 після T042; T053 від T044–T052, T054 після T053. T023 доповнює T019 після T021/T022.
- T022 фіксує єдину конкретну canonical timing archive schema та D76 event lifecycle до collector і T046/T047 fixtures; T050/T051 використовують її без зміни перед formatter/parser interoperability у T053; це деталізація selected embedded contract, не нова продуктова політика. Немає web/network schemas.
- T055–T057 після T054; T058 після них. T059 — readiness перед будь-яким physical run; T060 має бути визначений для джерела кожного run. HJ-only branch може рухатися з явним deferred receiver gate; actual receiver/Matek branch без його доказу не рухається.
- T061–T068 потребують T059 й застосовного source gate T060, exact binaries/config review та явного operator завдання. Рекомендований фізичний порядок — startup→mapping→Test→Fly→technical faults→watchdog→accuracy→stress; за помилки зависиму гілку зупинити, межі не послаблювати.
- За hardware blocker дозволено готувати проміжну частину T069/T070 з not_run/blocked й owners; checkbox задачі, що вимагає прийняття, лишається відкритим. Software completion та full acceptance — різні підсумки.

## Приклади паралельної роботи

Це приклади розподілу файлів після спільних prerequisites; команди не запускають hardware або агентів.

| Історія | Prerequisites | Незалежні test tasks | Послідовний merge/verification |
|---|---|---|---|
| US1 | T008 | T009 startup fixtures; T010 capture fixtures; T011 CLI fixtures | T012–T016; T017 реєстрація/запуск |
| US2 | T017 | T018 timeout fixtures; T019 owner-race fixtures | T020–T023; T024 реєстрація/запуск |
| US3 | T024 | T025 Fly fixtures; T026 resync fixtures | T027–T030; T031 реєстрація/запуск |
| US4 | T031 | T032 failures; T033 retry/D75; T034 supervision; T035 clock | T036–T041; T042/T043 host/build |
| US5 | T043 | T044 formatter; T045 transport; T046 collectors; T047 offline fixtures | T048–T052; T053/T054 results |

T030 має окремий test file, але позначення[P] не додається: його cases звіряються з уже інтегрованими T027–T029. Різні історії не оголошуються незалежними writers одних core/output/owner файлів. Фізичні runs не паралелізуються на одній платі/приладі.

## R/FR/SC → задачі → evidence

Усі `verification/` шляхи нижче — майбутні результати фактичного виконання; зараз їх не створено й pass не заявлено. H=`verification/host.md`, B=`verification/build.md`, C=`verification/config-review.md`, P=`verification/hardware.md`, A=`verification/acceptance.md` у цьому feature.

| Вимоги / критерії | Implementation і перевірки | Evidence |
|---|---|---|
| R01,R03,R10 /FR-001 /SC-001,002,008 | T004,T008,T010,T012–T017,T055–T057 | H/B/C/A; native core/one channel/no DIR/radios |
| R07,R11 /FR-002 /SC-002 | T011,T015,T017,T041,T052,T056,T057,T061 | H/B/C/P/A; ordinary й explicit selection |
| R07,R08 /FR-003 /SC-002 | T009,T012–T017,T025,T031,T061,T066 | H/B/P/A; startup/first-high/reset |
| R04,R05,R06 /FR-004 /SC-001,007 | T003,T009,T012,T014,T017,T055,T062 | H/B/P/A; unchanged map/rounding/endpoints |
| R05,R07,R09 /FR-005,006 /SC-001,003 | T010,T012,T013,T018–T024,T038,T047,T051,T063,T068 | H/P/A; last-valid/time/boundary/direct1с |
| R07,R09 /FR-007 /SC-004,007 | T025–T031,T064,T068 | H/P/A; long hold0/80/100 і нова high/low |
| R05,R07,R08 /FR-008 /SC-001,004 | T004,T010,T012,T013,T018–T021,T025–T028,T063,T064 | H/P/A; new valid only/resync/automatic recovery |
| R05,R07,R09,R11 /FR-009 /SC-004,006 | T003,T013,T021,T026,T028–T031,T055,T067,T068 | H/C/P/A; latch migration/input accounting |
| R06,R08,R09,R11 /FR-010 /SC-005 | T005,T008,T014,T019,T021,T032,T033,T036,T037,T042,T065 | H/C/P/A; checked output/retry D75 |
| R05,R08,R11 /FR-011 /SC-005 | T006,T030,T035,T038,T042,T065,T066 | H/C/P/A; clock vs old sample |
| R08,R09,R11 /FR-012 /SC-002,005 | T006,T007,T016,T034,T039–T043,T057,T066,T068 | H/B/C/P/A; actual progress/reset path |
| R09,R11 /FR-013 /SC-006 | T044,T045,T048,T049,T052–T054,T057,T068 | H/B/C/P/A; bounded latest-only diagnostics |
| R01,R08,R12 /FR-014 /SC-003–008 | T001,T054,T058–T070 | P/A; readiness/source/no-pump/gates |
| R04,R05,R06,R09,R12 /FR-015 /SC-003–008 | T022,T023,T040,T043,T046,T047,T050–T053,T057–T068 | H/B/C/P/A; bounded evidence/U/provenance |
| R12 /FR-016 /SC-008 | T001–T003,T008,T017,T024,T031,T042,T043,T053–T057,T069,T070 | H/B/C/P/A; commands/status/hash/traceability |
| R07,R08,R12 /FR-017 /SC-008 | T001,T054,T069,T070 | A; constitution2/D74/D75/D10, scope/auth |

## Стратегія виконання

1. **Перший програмний зріз** — T001–T017: спільна основа й US1 з host/build evidence. Зупинитися для оцінки software результату; не експлуатувати частковий образ і не оголошувати ready05/06.
2. Додавати US2→US3→US4→US5, після кожної історії запускати meaningful boundary/failure suites й регресію фактично зміненої поведінки. Не відновлювати duty через reboot, не вводити extra arming, manual input-loss clear або runtime mode.
3. Перед фізичними діями виконати fresh T055–T058. T059–T068 — окремі operator задачі за фактичною readiness. Недоступний прилад/reset anchor/U означає blocker відповідної метрики, а не default або host replacement.
4. Для Test незмінні T=500000мкс/age≥T, checked budgets і physical≤1с; Fly hold/automatic recovery не перетворюються на universal fault tolerance. D75 застосовується тільки до technical output episode, не input-loss resync.
5. Full acceptance05 потребує всіх обов'язкових доказів; software-only звіт залишає відкриті physical tasks. Продовження06, інші фази, Git publication й hardware дії не запускаються цим списком.

## Перевірка генерації

- `setup-tasks.sh --json` виконано з явно заданим `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming`; повернув абсолютний FEATURE_DIR цього iteration й TASKS_TEMPLATE=`/Users/mac/Projects/Scopa/rc-pwm-bridge/.specify/templates/tasks-template.md`. Доступні research/data-model/contracts/quickstart прочитано; spec/plan існують і належать05.
- Before/after Tasks hooks: `.specify/extensions.yml` відсутній; dispatch немає. Наступну фазу не викликано.
- Плановий обсяг:70 unchecked tasks; Setup3, Foundation5, US1=9, US2=7, US3=7, US4=12, US5=11, Final16. Позначено17 незалежних[P] задач у межах вказаних груп.
- Документальну валідацію виконано:70 послідовних T-ID, усі unchecked, правильні US labels/конкретні файлові шляхи/R або FR references;17[P] у6 групах без спільних редагованих файлів, усі17 FR і11 запитаних R-ID у matrix. Усі8 Markdown-посилань мають наявні targets. Whitespace check не знайшов помилок; hash-порівняння підтвердило незмінність вхідних tracked файлів, Git branch/HEAD збережені. Це не запуск host/target/hardware задач.
