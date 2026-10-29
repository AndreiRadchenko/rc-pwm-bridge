# План реалізації: 03-rc-capture

**Дата**: 2026-10-29 | **Spec**: [spec.md](spec.md)  
**Каталог**: SPECIFY_FEATURE_DIRECTORY=specs/03-rc-capture  
**Гілка**: 03-rc-capture, без перемикання в цій фазі.  
**Стан планування**: Phase 0 і Phase 1 завершено як дизайн D51/D52. Подальший Implement виконав software/host/target builds; актуальні outcomes — [verification/acceptance.md](verification/acceptance.md), physical not_run. Нижче збережено критерії та стан саме на момент Plan.

## Summary

Один RC-вхід HJ Tester 50 Гц на ESP32-C3 SuperMini, native ESP-IDF. RMT вимірює ширину, GPIO ISR записує оцінений час фронтів. Після однозначного зіставлення width і period перевіряються перед accepted update ядра; last-valid отримує estimated falling time. Окремий monitor визначає незмінний HIGH/LOW після 100 мс. GPIO0 лишається LOW, permission false. I2S/hardware timestamp більше не потрібні за D51.

Конституція v1.0.0 заповнена й ухвалена 2026-10-28; це не stock placeholder. Програмні докази 01/02 прочитані: 02 має host 8/8 і target build, але app ще не викликає ControlCore. Фізичні H02/H03 01 відкриті. PROJECT_FOUNDATION.md — історія; WROOM/двоканальний старт не чинні.

## Technical Context

| Пункт | Вибір |
|---|---|
| Мова | Portable core/capture policy C++11, без IDF/FreeRTOS/I/O/heap; ESP32 adapter/app C++17. Без RTTI/exceptions. |
| SDK | Native ESP-IDF v5.5.6, commit880a203ae603d08a94d06aa01e0d6761958e5e9e; modern driver/rmt_rx.h. Нових сторонніх залежностей немає. |
| Tools | GCC14.2.0 esp-14.2.0_20260121; Python3.11.7; CMake3.30.2; Ninja1.12.1; host Apple clang21.0.0. Хеші та локальні шляхи — tools/environment/environment.lock.json; перевіряти перед запуском. |
| Профіль | firmware/boards/c3-supermini-pinout-01.json, D18–D20: C3 QFN32 rev v0.4/4MB, RC GPIO3, output GPIO0, DIR–GND апаратно. Фізичної придатності build не доводить. |
| RAM | Bounded static queues/buffers, saturating counters; без NVS/історії/endpoint learning. IDF resources виділяються один раз при init, не в capture ISR чи steady state. |
| Перевірки | Portable fake-clock fixtures, окремий target build/config/map, окремий tester/analyzer acceptance. Усі нові результати not_run. |
| Межі | Один вхід, 50 Гц; width 900–2100 мкс, period 18000–22000 мкс включно. Output PWM04, operational safety 05, насос 06, два входи 07. Немає Arduino/radio/ignition. |

### Технічні параметри Plan (D52)

Це вибори дизайну до реалізації й вимірювання, а не результати фізичної перевірки.

| Параметр | Значення / причина |
|---|---|
| RMT | RMT_CLK_SRC_APB, resolution_hz=1000000: tick1 мкс, 15-bit duration до 32767 мкс. PM/DFS/light sleep вимкнені в 03. |
| Stop | signal_range_max_ns=5000000: 5 мс вище valid HIGH2,1 мс і заявлених 2,2 мс тестера, нижче min valid LOW15,9 мс. Це межа транзакції, не stuck timeout. |
| Filter | signal_range_min_ns=0; smoothing/debounce немає. Короткі вимірювані invalid pulses не приховуються. Субмікросекундна активність може бути нероздільною; ambiguity не дає valid. |
| Buffer | mem_block_symbols=48, scratch48 symbols=192 B internal SRAM, with_dma=false, en_partial_rx=false. Final callback копіює ≤48 symbols, metadata, перевіряє enqueue і rearms; не форматує/не викликає core. |
| Черги | 64 GPIO edges та 8 RX batches по≤48 symbols; static storage. Sticky error/counters поза чергами, щоб queue overflow не приховав fault. |
| ISR | GPIO ANYEDGE і RMT interrupt priority3; handlers/data у IRAM/internal SRAM. CONFIG_RMT_RX_ISR_CACHE_SAFE=y, CONFIG_RMT_RECV_FUNC_IN_IRAM=y, CONFIG_GPIO_CTRL_FUNC_IN_IRAM=y. Effective config і весь call graph перевіряються в map. |
| Scheduling | Один capture/control owner priority10; diagnostics priority1. esp_timer task-dispatch1000 мкс лише notify owner. CONFIG_FREERTOS_HZ=100 не використовується як точність monitor. |
| Rearm | ≤1000 мкс від очікуваний stop=falling+5000 до успішного arm у нормальному потоці. Min LOW18000−2100=15900; запас 15900−5000−1000=9900 мкс до наступного valid rising. |
| Processing | ≤10000 мкс від RX completion до validation/core update. Прострочення — counter/fault/resync, не підміна edge time на now. |
| Stuck | Threshold100000 мкс від останньої зміни рівня/startup; додатково≤10000 мкс до sticky fault у snapshot/core, тобто не пізніше age 110000 мкс. Текст може з’явитися пізніше. Це не Q05 cutoff. |
| Active guard | RX без final після першого rising12000 мкс — abort/fault/resync у додаткові 10000 мкс. Arm без фронтів guard не запускає; його контролює stuck monitor. |
| Pairing sanity | Єдина rising→falling пара того самого epoch/sequence window; abs(GPIO duration−RMT width)≤50 мкс як додаткова технічна rejection boundary. Це не заміна допусків 5/10 мкс і не width-based пошук пари. |
| Diagnostics | Один latest pending snapshot; максимум один уже committed USB record ≤384 B; TX ring512 B. Перед write — wait_tx_done(0), одне write(...,0) на слот≥100 мс. Старий текст не повторюється. |

### Рішення і відкриті gates

Q04 програмні параметри 03 визначено D41/D43/D45/D46/D49/D51/D52; blocking technical unknowns для Phase1 немає. Фактичні latency/accuracy/loss ще мають бути перевірені. D53 визначає обсяг фізичного прийняття; правило прийняття задано D54/CHK026-B: пряме E≤5/10 мкс, U≤1 мкс окремо без запасу.

Q03 electrical conditioning/ground/protection і напруги HJ Tester — до підключення; analyzer/timing/loss — до фізичного прийняття. H02/H03 01 не скасовано. За D48 receiver перевіряється до використання, не блокує прийняття 03 із HJ. Джерела: [acceptance01](../01-foundation/verification/acceptance.md), [acceptance02](../02-control-core/verification/acceptance.md), [prerequisites02](../02-control-core/verification/prerequisites.md).

Q05 operational timeout/cutoff/arming/recovery/watchdog лишається у 05; його 100 мс — досі лише історична пропозиція. Q06 силовий тракт належить 04/06. Немає нового watchdog/reset policy у 03.

## Constitution Check

| Gate | До research після D51 | Після Phase1 |
|---|---|---|
| I — Scope | pass: один input, radios off. | pass: capture/diagnostics03; карта 02 і майбутні 1000 Гц output не змінені. |
| II — Safety | pass для дизайну: LOW/disarmed, Q05 окремо. | pass для дизайну: startup/fault/resync і budgets явні; watchdog/cutoff 05. LOW MD20A — brake. Hardware acceptance blocked H02/H03/Q03, power isolation окремо. |
| III — Architecture | pass: portable core/native IDF/profile. | pass: portable reducer і platform adapter, units/provenance, один owner; інші плати не оголошені підтриманими. |
| IV — Evidence | pass для плану: pins01/02, категорії evidence. | pass для плану: limits/traceability/validation guide до вимірювання; нові tests not_run. Physical acceptance blocked окремо. |
| V — Workflow | pass: лише Plan, явний каталог 03. | pass: setup-plan returned paths саме 03; user files збережені; Tasks/Implement/commit/flash/load не виконувались. Документація українською. |
| VI — Decisions | pass: D51 явно замінив hardware latch/3 мс. | pass: D52 явні technical choices; Q03/Q05/Q06 не приховані, точність/придатність не оголошено доведеними. |

## Архітектура та порядок роботи

~~~text
GPIO3 → RMT RX → final callback → bounded RxBatch queue ┐
      → ANYEDGE ISR → bounded EdgeEvent queue          ├→ capture/control owner
      → latest line + sticky errors                   ┘   → portable reducer → ControlCore 02
1ms timer → notify owner                                      → latest snapshot
snapshot → diagnostics worker → zero-wait USB                  output GPIO0 LOW
~~~

1. Startup 01 перевіряє профіль і застосовує LOW перед capture; ControlCore створюється disarmed. StartupPolicy зберігає власні retry/deadline, capture запускається один раз після успішного setup. Відсутній USB reader не блокує capture.
2. Створити черги/owner, channel/callback, GPIO ISR після RMT routing, initial level/time, monitor. Не reset GPIO3 після routing і не переналаштовувати output/DIR. LOW startup дозволяє arm: hardware очікує першу зміну рівня. HIGH startup — incomplete, чекати LOW, паралельно monitor100 мс.
3. Arm створює epoch/arm_seq під коротким спільним critical-section протоколом GPIO/RMT/owner; LOW/seq/pending interrupt перевіряються навколо arm, concurrent boundary дає ambiguity/resync. Recovery_requested блокує callback rearm під час task recovery; докладно — контракт. ISR записує seq/epoch/level/estimatedtime і незалежний latest level/time. Final callback копіює batch+close_seq перед повторним використанням scratch і rearms уLOW. Якщо HIGH/API failure — очікування LOW/відновлення owner, без сліпого arm посеред HIGH.
4. Owner зіставляє batches із edges за epoch/window: рівно одна complete HIGH-пара, правильні levels/terminator і sanity check. Потім width/period validation; перший period unknown; invalid period переносить origin до останнього rising заD49. Invalid не змінює last-valid і сама не latchfault.
5. Будь-який final callback очищує active guard старого epoch; новий guard починається з першого rising нового epoch. HIGH≥5 мс не має повної RMT width: incomplete/overrange, stop не є falling, end не вигадувати. Чекати LOW; саме це не capture fault до 100 мс. Повні invalid width нижче stop — invalid, не latch. Реальна втрата меж має окрему fault причину.
6. Overflow, gap/ambiguity, saturated final num_symbols ≥48, clock/API/rearm error, processing/transaction overrun — sticky reason/counter, barrier, discard/resync. На GPIO queue overflow ISR маскує interrupt через IRAM-safe control, будить owner; той abort/reset/re-enable уLOW і скидає period origin. Потрібні два нові rising; latch core не очищується.
7. Core calls лише owner, нормальні valid FIFO за estimated end. Fault застосовується з now; queued pre-barrier results відкидаються й рахуються без переписування end. Якщо можна bounded drain завершених старих valid перед fault без порушення budget — обробити їх раніше. Нові valid після barrier можуть оновити last-valid під latch. Core TimeOrderError — додатковий capture fault.
8. Monitor читає level/time/seq узгоджено й повторно перевіряє seq перед stuck, щоб concurrent edge не дав falsefault. Один stuck event на епізод незмінного рівня; counters не збільшуються на кожному poll. Clock 64-bit не звужується; sequencewrap модульний у bounded window.
9. Owner формує snapshot; diagnostics окремо. Немає production clear/arm command. Init failure зберігаєLOW/fault; повторний resourceinit лишеowner не частіше 100 мс, без очищення latch. Capture/core не чекають serial.

## Verification and Traceability

H/B/P — назви майбутніх сценаріїв, не implementation tasks. Всі нові результати 03 not_run.

| R / FR / SC | Перевірки | Майбутній evidence |
|---|---|---|
| R05 / FR-001–003,006 / SC-001/002 | H1 width 899/900/1000/1250/1400/1500/1800/2000/2100/2101; period 17999/18000/22000/22001; перший period unknown; validate-before-clamp; H2 startup/missing edge/HIGH overrange/stuck/resync/latch. | verification/host.md: reducer+core, fake clock, без MCU headers. |
| R09 / FR-001–004,006 / SC-001–003 | H3 seq/epoch/reorder/drop/saturation/rollover/clock regression/barrier, concurrent arm/recovery; P1/P2 rearm≤1 мс, processing≤10 мс, fault≤110 мс під навантаженням діагностики. | verification/host.md, verification/hardware.md. |
| R11 / FR-004 / SC-003/004 | H4 10/с, ≥100 мс, overwrite/drop/counters, slow/absent reader/reconnect, один in-flight record/partial write/без історії; P2 максимальний фактичний payload. | verification/host.md, verification/hardware.md. |
| R12 / FR-005,007 / SC-003/004 | B1 native C3 build/config/map/IRAM call graph; P1 кожна width≤5 мкс, rising interval≤10 мкс; P2 zero втрат valid/600 с 50 Гц один вхід/output LOW. | verification/build.md, verification/hardware.md; electrical gates pending. |

Analyzer після conditioning наGPIO3. Без clocksync порівнювати rising intervals. Countexpected із analyzer у спільному стабільному 600 с вікні, не припускати рівно 30000. Rearmloss усередині вікна рахувати; recovery окремо. Невідповідність — fail, не автоматичне послаблення tolerance.

## Project Structure

Наявні control/startup/profile/tooling/tests зберігаються; плановані файли:
~~~text
firmware/components/bridge_core/include/bridge/rc_capture.hpp  # new portable DTO/policy
firmware/components/bridge_core/rc_capture.cpp                 # new reducer/parser
firmware/components/platform_esp32/include/platform/rc_capture.hpp
firmware/components/platform_esp32/rc_capture.cpp              # new IDF adapter
firmware/components/platform_esp32/local_diagnostics.cpp       # adapt D43
firmware/components/platform_esp32/startup_policy.cpp         # portable diagnostics policy D43
firmware/components/platform_esp32/include/platform/startup_port.hpp
firmware/components/platform_esp32/include/platform/local_diagnostics.hpp
firmware/main/Kconfig.projbuild                              # default-off bench option
firmware/sdkconfig.rc03-bench.defaults                        # bench fragment, baseline preserved
scripts/build.py                                             # explicit target-only bench flag
scripts/verify/config.py                                     # effective config/graph checks
tests/tooling/test_build_contract.py                          # build/config negative fixtures
firmware/main/app_main.cpp                                    # compose, keepLOW
firmware/sdkconfig.defaults                                  # IRAM/cache/PM
firmware/components/*/CMakeLists.txt                          # RMT, hostsources
firmware/components/bridge_core/include/bridge/rc_capture_trace.hpp
firmware/components/bridge_core/rc_capture_trace.cpp            # portable bounded bench collector
scripts/verify/rc_capture_trace.py                             # offline comparison, stdlib only
tests/host/rc_capture_trace_tests.cpp                          # bounded collection/freeze
tests/tooling/test_rc_capture_trace.py                        # fixed reference comparison fixtures
tests/host/                                                  # other new fixture suites
~~~

Phase1: data-model.md, contracts/rc-capture.md, quickstart.md. Research і timestamp-alternatives збережено з історією. Verification records створюються при фактичних перевірках. Tasks.md уPlan не створюється.

## Compatibility and Complexity Tracking

ControlCore API/поведінка 02 не змінюються; periodvalidation — передумова адаптера. DiagnosticSnapshot/текст розширюються; consumer має читати complete marked records. Один committed USB record не можна відкликати; після його завершення наступним є latest state у звичайному темпі. Історії pendingrecords немає.

| Компроміс | Чому потрібний | Простіша альтернатива |
|---|---|---|
| RMT+GPIO ISR | RMTwidth і estimatededge time наC3. | GPIO-only відступає від RMTscope; hardwaretimestamp/I2S складність знятаD51. |
| Epoch/sequence queues | Не переплутати пізній result із наступнимpulse. | Пошук за схожоюwidth може підставити чужий час. |
| Onein-flight USB | Не накопичувати історію вdriver. | Zero-waitwrite безdrain gating сам по собі не latest-only. |

Винятків із конституції немає; завершення дизайну не означає завершення ітерації чи дозвіл на hardware.


## Уточнення checklist 2026-10-29

- Expected HIGH stop та unexpected data truncation розрізнено у spec/contract. Invalid period не скасовує окремого resource guard/error.
- Owner враховує також завершені stuck episodes; missed deadline дає deadline_missed/fault/lateness і failed evidence. Новий edge не стирає попередній ≥100 мс епізод.
- Після ISR masking немає доказу безперервності спостереження: observation_continuous=false; recovery започатковує новий interval без вигаданого edge, зберігає latch/last-valid і вимагає resync.
- Одиниці diagnostic counters та version-specific ack визначено в контракті; overwrite не дублюється читанням, busy-slot і write-drop не дублюють один слот.
- Max diagnostic bench load: record 384 B з актуальними полями та дозволеним padding, спроби через 100 мс; при backpressure записуються фактичні bytes/drops.
- H2/H3 додатково охоплюють edge після threshold до monitor, deadline miss, невидимий інтервал masking/re-enable. H4 — concurrent publish/ack та точні одиниці counters.

## Рішення власника щодо acceptance scope

D53 / CHK025-A (2026-10-29): для завершення 03 точні межі періоду 18/22 мс, точні граничні ширини та складні malformed-послідовності покриваються host fixtures. Фізично обов’язкові HJ Tester на 50 Гц у доступному діапазоні, width/interval accuracy, zero loss під diagnostic load, stuck HIGH/LOW та rearm/processing/fault budgets. Окремий генератор цих точних stimuli не блокує 03; host не підміняє зазначені обов’язкові фізичні докази. Q03/H02/H03 і receiver gate зберігаються.

## Рішення власника щодо правила вимірювання

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

## Аналіз узгодженості 2026-10-29

E1: lifecycle fixtures у T021 явно покривають init failures/partial cleanup/retry без дубльованих ресурсів, які реалізує T027. I1: Project Structure синхронізовано з конкретними файлами T034/T037/T040 без нового feature scope. C1 закрито вибором A/D55: короткі RAM accuracy windows + окремі 600 с loss/timing tests; деталі нижче.

## Метод фізичного accuracy evidence — D55 / C1-A

D55 / C1-A (2026-10-29): точність перевіряється в коротких вікнах із bounded RAM trace всіх імпульсів цього вікна та вивантаженням після нього. Окремі 600-секундні тести перевіряють втрати й часові бюджети під max diagnostics. Допуски 5/10 мкс та U≤1 мкс окремо за D54 незмінні; середнє не замінює per-pulse/interval порівняння. Production diagnostics не стає історією вимірювань.

Технічний вибір Plan: 128 records × максимум 48 B =6144 B, header/метадані ≤256 B; разом додаткова trace RAM≤6400 B, exporter використовує bounded buffer≤384 B. Це upper bound для static assertions/map review, не вже виміряна доступна RAM. Portable collector C++11 без I/O/heap у rc_capture_trace.hpp/.cpp; platform owner копіює результат, не ISR.

Trace option default off, окремо від diagnostic-stress option. Потрібні три build variants: normal, diagnostics-stress (600 с, trace off), accuracy-trace+diagnostics-stress. Під час короткого вікна reader active/slow/absent та diag 384 B/100 мс не змінюють збір; усі 128 послідовних candidates з width/status/estimated start/end/seq/epoch збираються без перезапису. Перший unknown period не видаляється. Перед вікном фіксуються baseline counters, наприкінці — delta/flags/max latencies; latch і реальні counters не скидаються. Новий capture drop/fault/ambiguity в межах вікна або насичений counter не дозволяє оголосити повне вікно успішним.

На 128-му record нормальний full означає freeze, не overflow. End window = end останнього record, а не пізніший час dump; події після закритої межі не додаються в trace. Owner зупиняє capture для export, outputLOW/permissionfalse та fault latch не змінюються. Одна bench session збирає одне вікно; наступна — лише окремий операторський restart.

Після freeze sole USB writer може повторювати той самий bounded archive у циклах HEADER/128 indexed records/END, ≤10 frames/с, ≤384 B/frame, one-inflight і zero-wait. Це explicit bench-only archive phase з capture_active=false, без production snapshot replay або нового command input. Повторні цикли потрібні після відсутнього reader: host приймає лише повний незмінний цикл, не partial data; memory не росте. Режим експорту відокремлений від capture window, не підміняє 600 с stress.

Offline comparator — Python3.11 stdlib csv/decimal/json/hashlib, без нових dependencies. Він читає повний archive та нормалізований analyzer CSV (pulse_start_ns,pulse_end_ns, цілі ns). Vendor export зберігається з hash, нормалізація документується; невідомий формат конкретного приладу не вигадується. Порівнювати RMT width з analyzer width, а rising-to-rising intervals — між GPIO start та analyzer start; без absolute clock sync, offset/drift correction або заміни RMT width на end−start. E не округляється перед прямим D54 comparison.

Для відповідності імпульсів починати з перевіреного LOW і відомої межі першого імпульсу: analyzer увімкнено до подачі RC, firmware armed/waiting signal підтверджено до зміни reader condition. Порівнюються послідовні records у закритому вікні, з count/order/relative timing checks. Невідповідне зіставлення або відсутній record — fail/inconclusive, не best-fit чи видалення outlier.

Виконати короткі вікна по доступному HJ діапазону й reader conditions під max diagnostics; кожен pulse/interval в цих вікнах має пройти D54. Окремі 600 с runs підтримують тільки zero-loss/counts/latency maxima evidence: жодного твердження про accuracy кожного імпульсу за всі 600 с. Усі operator gates D53 зберігаються.

## Уточнення входу D56,2026-10-29

Після кожного успішного створення RMT RX-каналу адаптер задає GPIO3 GPIO_FLOATING, вимикаючи внутрішні pull-up/pull-down, які інакше залишає SDK. Виклик перед callbacks/enable, результат перевіряється; помилка видаляє створений канал і повертає init failure у наявний fault/retry lifecycle. Зовнішній дільник задає bias. Це прямо погоджена програмна зміна; фактичні HJ рівні/фронти, номінали дільника й Q03/H02/H03 залишаються фізично pending. GPIO0, карта/пороги capture, core02/arming не змінюються.

## Уточнення USB driver contract після реалізації

TX ring512B збережено. RX ring має128B: ESP-IDF5.5.6 driver install вимагає RX>64B; початкові64 відхилялися. Application не використовує RX як command source. Regression lower-bound test додано; фізичний startup diagnostic output після виправлення pending.
