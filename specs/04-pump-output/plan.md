# План реалізації: 04-pump-output

**Ітерація**: `04-pump-output` | **Дата**: 2026-10-30 | **Spec**: [spec.md](spec.md)\
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output`\
**Гілка**: `04-pump-output`, без перемикання у Plan.\
**Стан**: Phase0 та Phase1 завершено як дизайн за D59/D60. Tasks/Implement не запускались; нові host/build/physical результати04 відсутні.

## Короткий опис

Один вихід GPIO0 на ESP32-C3 SuperMini: LEDC1000Гц при проміжних duty, static LOW/HIGH на0/100%. DIR постійно з'єднаний із GND на MD20A без MCU GPIO. Звичайний образ04 лишається LOW/disarmed. Окремий no-pump образ після кожного app boot може одноразово виконати0/25/40/50/80/100%, повернутися в LOW й не повторюватися до наступного boot. Capture/core та bounded diagnostics03 збережені; operational arming/freshness/watchdog —05.

Phase0 підтвердив неможливість реалізувати power-cycle-only D58 через reset enum C3. Користувач явно обрав іншу bench-only політику D59; [research RCH-01](research.md#rch-01--перевірка-виконуваності-d58) зберігає доказ і відхилені альтернативи. Це усуває блокувальний програмний вибір без нового GPIO/command source. Firmware не визначає фізичну ізоляцію, оператор забезпечує від'єднаний насос протягом усіх boot. Нормальна збірка не набуває operational permission.

## Технічний контекст

| Пункт | Вибір / межа |
|---|---|
| Мова | Portable OutputPolicy C++11 без IDF/FreeRTOS/GPIO/I/O/heap; app/platform C++17, наявні no RTTI/exceptions.02 ControlCore API не змінюється. |
| SDK/tools | Native ESP-IDF v5.5.6, commit880a203ae603d08a94d06aa01e0d6761958e5e9e; GCC14.2.0 esp-14.2.0_20260121, Python3.11.7, CMake3.30.2, Ninja1.12.1; host Apple clang21.0.0. Хеші/локальні шляхи — tools/environment/environment.lock.json, перед збіркою recheck. |
| Залежності | Native esp_driver_ledc плюс наявні gpio/RMT/timer/USB. Без сторонніх бібліотек/fade/radio/network/NVS feature. |
| Профіль | c3-supermini-pinout-01/unit c3-supermini-188b0e93032c; PWM GPIO0, RC GPIO3, DIR hardware GND; зовнішні3,3В на3V3/GND. Фізична придатність pending. |
| LEDC | Low-speed timer0/channel0, APB80МГц,13bits,1000Гц,hpoint0,invertfalse,no fade/no sleep. Divider2500/256 точний; endpoints через stop idle0/1 без duty8192. |
| Пам'ять | Незмінний сценарій6 points, маленький OutputPolicy/Snapshot; normal без evidence history. Bench OutputEvidence≤2688B; разом з accuracy trace03≤9088B. Stress має окремий CaptureWindowEvidence≤512B, разом≤3200B; accuracy collector у stress відсутній. Actual map/stack виміряти при реалізації. |
| Час | Ті самі64bit monotonic мкс; owner notify1мс03, без очікувань/serial/flash у runtime path. Output deadline→LOW API≤2мс, owner fault→LOW API≤1мс, lateness≤1мс; не Q05. |
| Масштаб | Один input50Гц, один output1000Гц; pin/profile/core invariants01–03. Запалювання/інші ESP32/Nano —07–09. |
| Перевірки | Meaningful portable/adapter/tooling fixtures; pinned native builds/config/map; окремі analyzer/voltage/operator records. У Plan не виконувались. |
| Відкриті залежності | Q02/H02/H03, Q03 та Q06 revision/interface/jumper/живлення/bias/прилад — фізичні gates, не приховані defaults. Q05→05; напрямок/силовий тракт/навантаження→06. |

## Перевірка конституції

Конституція v1.0.0 прочитана. D10 є чинним branch exception лише для Specify; у Plan branch/commit/push/hardware mutations немає. Усі generated документи українською; prompts/installed skills/templates не переписані. Старі статуси roadmap/architecture не переважають фактичні [acceptance01](../01-foundation/verification/acceptance.md), [acceptance02](../02-control-core/verification/acceptance.md), [acceptance03](../03-rc-capture/verification/acceptance.md).

| Gate | До research | Після Phase1 |
|---|---|---|
| I — Scope | pass: один channel/no pump/native IDF/radiosoff | pass: R01/R04/R06, deferred07–09 excluded |
| II — Safety | pass для дослідження виконуваності D58; фізичні gates окремо | pass для дизайну за D59/D60: startupLOW, finite bench, terminal fault priority, deadlines/measurement method. Попередній blocked D58 усунуто явним рішенням, не waiver. Фізична придатність blocked Q02/Q03/Q06. |
| III — Architecture | pass: portable core/existing profile/explicit adapters | pass: єдиний GPIO0/LEDC owner, стандартнийC++11 OutputPolicy,01–03 behavior preserved; ніяких unsupported boards |
| IV — Evidence | pass: pinned lock, meaningful tests і M04 owner | pass для дизайну: M04 заданий до вимірювань, traceability H/B/P та окремі records. Фактичне обладнання/U/statuses ще pending, hardware pass не заявлено. |
| V — Workflow | pass: лише Plan, явний feature04, preserve work | pass: setup-plan повернув04; research agents read-only за skill; Phase1 docs only, Tasks/Implement/device/flash/load/commit/push не виконувались |
| VI — Decisions | pass: D57/D58 та Q-власники явні | pass: RCH-01 документує limitation, D59 явно замінює D58, D60 закриває technical M04. Q02/Q03/Q06 physical/Q05 later-owned не домислюються. |

Завершення дизайну не є зняттям фізичних gates або дозволом запускати стенд. Недоступний прилад/непідтверджена ревізія не блокують оформлення дизайну, але блокують залежне підключення/прийняття. Конституційних винятків, крім уже чинного D10 для минулого Specify, не вводиться.

## Архітектура та порядок роботи

1. app створює незмінний BenchConfig, один EspPumpOutput та існуючі startup/capture/diagnostics. Constructors не вмикають nonzero. Profile/chip/flash перевіряються в init, не в2мс output path.
2. StartupPolicy використовує LOW-метод того самого output adapter; зберігає D15 retry count/deadline. Нове поле startup_had_failure фіксує будь-яку невдалу init-спробу для bench gate. Readiness після retry не знімає цю заборону в поточному boot.
3. На ReadyDisabled одноразовий handoff передає output/control ownership існуючому capture owner. До цього GPIO0LOW; після handoff app не пише GPIO/LEDC. capture.start idempotent/retry не створює нову OutputSession. Помилка capture/resource/timer start є чинним latch та позбавляє boot nonzero eligibility.
4. Owner на першому poll фіксує t0 та initial LOW5с. Bench config і startup context immutable. На fixed deadline за readiness без fault починається Step0; очікування RC/reader не переносить start_at і не створює production arming. Відсутній RC дає чинний03 stuck fault і припинення, а не його маскування.
5. На початку step owner спершу знімає pending sticky errors/losses03, застосовує їх до capture/core/output і встановлює fault barrier; до цього nonzero output заборонений. Сервіс OutputPolicy виконується після кожного policy fault/on_edge/on_batch/processing_complete/poll та перед recovery/publish, із точкою серіалізації sticky flags під коротким lock до кожного nonzero API, включно зі stop idle1 для100%. Lock не охоплює SDK call; ingress після цієї точки повторно перевіряється одразу після операції, з fault/LOW перед наступним nonzero API. Зокрема recheck обов’язковий між set і update; атомарного ISR-ingress→physical write не обіцяємо. Операції drain64edges/8batches та budgets03 збережено; output service не чекає повного drain або app10мс poll. ISR тільки повідомляють owner.
6. Переходи за абсолютними deadlines, без catch-up: overdue>1мс→Fault/LOW. Adapter застосовує тільки змінену generation. Точні set/update/stop/fallback/cleanup та checks — [контракт](contracts/pump-output.md#апаратний-adapter). Немає fade service/інших LEDC clients, runtime allocation або profile flash reads у критичному шляху.
7. Fault/Complete заморожують право ненульового output для boot. Capture resync/retry/valid продовжують свої contracts;02 clear_fault не експонується. Якщо фізичнийLOW не підтверджений через API failure, зберігається LowUnconfirmed/failed evidence, а не помилковий cutoff success. Зависання task не видається за виконаний deadline; deadline miss — fail.
8. Один узгоджений publish capture/output snapshot, діагностика окремою task priority1. Core calculated/effective0/permissionfalse і requested bench/applied-status розрізнені; output faults/counters sticky. Capture ownerpriority10/IRAMcritical sections/GPIO3floating/D56 не змінюються. Hardware LEDC не потребує software update на кожний PWM-cycle.
9. На reset старий крок зникає, початковий LOW і нова сесія за D59; reset reason лише informational. Не додаються reset handlers, які обіцяють LOW раніше firmware; апаратний bias Q02/Q06 покриває цей інтервал.

## Параметри M04 та evidence builds

Нормативна повна таблиця/правило методу — [pump-output.md](contracts/pump-output.md). Всі числа — технічний вибір Plan за D60 до реалізації.

| Пункт | Значення |
|---|---|
| Normal | Постійний LOW/disarmed, bench default off |
| Waveform | Initial LOW5с +6×10с =65с, даліLOW; stable windows+1..9с, endpoints весь plateau |
| Stress | InitialLOW5с +6×100с=605с; окреме600с вікно відt0+5с доt0+605с; max diagnostics384B/100мс, active/slow/absent reader у різних runs |
| Accuracy | Waveform sequence; збір128 candidates03 починається на+1с вибраного trace step. На freeze LOW перед stop capture/export. Не є full-sequence або600с evidence. |
| Duty counts |25/40/50/80% →2048/3277/4096/6554, round halfup; всі1..99% тестуються.0/100% static stop0/1. |
| Timing | MCU clock: lateness≤1мс, deadline→API result≤2мс, applied owner fault→LOW API≤1мс. Окремо фізичний GPIO3-stuck→GPIO0-LOW≤112мс; фізичні plateau intervals: E≤0,1% від номінальної тривалості+2мс, U≤0,01% тривалості+0,1мс. Cross-domain owner→physical2мс не заявляється. |
| Frequency/duty | Per-cycle E_f≤1Гц/U_f≤0,1Гц; E_d≤0,1п.п./U_d≤0,02п.п., без averaging/rounding для pass |
| Voltage | LOW+U≤0,8В; HIGH−U≥2,4В, HIGH+U≤3,6В; U_V≤0,05В. No proof without compatible voltage method/ground/power. |
| Detection | Протилежний рівень≥1мкс має виявлятися; digital≥10MS/s; analog≥10МГц/20MS/s або доведена еквівалентна методика; actual U/gaplessness gate |
| Startup |≥5повторів кожного cold-start/reset/bootloader/upload у disconnected-driver і powered-driver conditions; pretrigger≥10мс, full startup/initialLOW; bootloader interval≥10с |

## Вимірювання та compatibility

- До waveform tests потрібні H02/H03/живлення/bias та прийнятий інтерфейс конкретного MD20A без мотора; HJ використовувати лише після Q03. D48 дозволяє HJ-only capture acceptance, не вимагає невідомий receiver; receiver має gate до використання.
- Existing D55 accuracy exporter не можна заповнювати від startup і зупинити capture до5с initialLOW, а потім оголосити accuracy при активномуPWM.04 trace-step gating переносить лише старт bench collector; live capture/validation до цього працює. Freeze спочатку вимагає checked LOW, потім навмисно зупиняє capture. LOW error лишає Fault/неприйнятний run, а LowUnconfirmed блокує normal evidence stop; overflow recorder не блокує LOW/fallback. Метод E5/10мкс/U≤1мкс та no selection128candidates незмінний. Зміна цього instrumentation має meaningful fixtures і evidence у04; історичні03records не переписуються.
- OutputEvidence64×40B/header≤128B записує API brackets/fault/deadlines безUSB у owner. Frozen export одним USB writer; жодного нового markerGPIO. API timing перевіряється у MCU clock, фізична форма/plateau intervals та112мс — напряму analyzer. Немає self-calibration clock offset через припущений output delay; cross-domain latency без незалежного anchor→blocked і не входить до заявлених pass04.
- Окремий rc04 diagnostic grammar усі значенняhex, ≤383B worst-case/≤384B padded; адаптувати consumer/tests. rc03-trace grammar і offline capture comparator збережені. Future05 має явно змінити o/a коли operational permission з'явиться;04 не імітує це.
- Output archive version1/FNV-1a64, порядок й identity — [data-model](data-model.md#outputevidence-тільки-bench). Це bounded bench-only history, не production diagnostics.

- D61 / CHK023–024-A: stress має окремий RAM-підсумок точного вікна [t0+5с,t0+605с), незалежний від reader. Облік за measurement.end_us включає пізно оброблені належні samples, а не різницю global snapshots. Після кінцевого checked LOW capture продовжує працювати; owner закриває підсумок після ingress barrier/drain у чинних бюджетах03. На справному потоці технічний guard закриття≤17мс після кінця: stop5мс + callback/rearm1мс + processing10мс + owner poll1мс; це вимога до реалізації, не виміряна гарантія чи послаблення кожного бюджету. Missed budget/невизначена асоціація дають failed/incomplete evidence. RAM≤512B; frozen summary експортується поряд із rc04-out одним writer/слотом після тесту. Схема/межі — data-model та контракт; normal/accuracy цього summary не містять.

Для H5/H6 додати meaningful fixtures: pre-window sample оброблений після start; end_us рівно на кожній межі; належний sample оброблений після end; ingress barrier проти callback; missed closure guard/late post-freeze sample; overflow/saturation/неоднозначний epoch; відсутній reader до завершення, пізній reconnect і повторний повний export без зміни frozen counts. У B1 перевірити окремі RAM межі stress/accuracy. Це обсяг майбутніх перевірок, не виконані тести.

### Перевірки та простежуваність

H/B/P — назви сценаріїв майбутніх перевірок, не tasks.md. Всі результати04 not_run.

| R / FR | Сценарії / SC | Перевірки / майбутній evidence |
|---|---|---|
| R01 / FR-001,FR-010 | US2.AC5,US3.AC6 / SC-003,006 | B1 profile/GPIO/безDIR; P1 revision/interface/ground/DIR jumper/buttons, board vsdriver traces; verification/build.md,hardware.md |
| R04,R06 / FR-002,FR-006 | US2 / SC-001–004 | H1 six points/1..99/counts/endpoints/>100 rejection; H2 HIGH↔PWM↔LOW; P2 nominal/stable/endpoints/transitions; verification/host.md,hardware.md |
| R08 / FR-003,FR-004 | US1,US3.AC5 / SC-003,004 | H3 fixed deadlines/reset-per-boot/no-repeat/latepoll/startupfailure; P3 boot/reset/upload/bootloader, full65/605с; normalLOW throughout; host.md,hardware.md |
| R08 / FR-005,FR-009 | US3.AC1–2,5 / SC-003,004 | H4 failure at everyinit/set/update/stop/fallback/cleanup step, stale generation afterfault, LowUnconfirmed, freezepriority; P4 explicitfault/stuck/outputLOW/MCU API timing і прямі physical intervals; host.md,hardware.md |
| R05,R09 / FR-006,FR-007 | US3.AC2–4 / SC-001,004,005 | H5 regression invalid/lastvalid/latch/timeorder/epoch/resync; P5 D53–D55 accuracy windows attrace steps/stress600с/rearm/processing/100+10/stuck→LOW112; host.md,hardware.md |
| R11 / FR-008 | US3.AC4 / SC-005,006 | H6 rc04 worst-case384B/version/enum bounds, latestack, absent/slow/reconnect/no extra command; boundedarchive count/order/digest/mapping/overflow negativecases; P5 maxload/no history; host.md,hardware.md |
| R02,R03,R10 / FR-012 | US1.AC1 / SC-005,006 | B1–B4 normal/waveform/stress/accuracy builds, config/map/IRAM/stack/coreheaders/fadeabsence; negative flags/normal nonzero off; verification/build.md,host.md |
| R12 / FR-010,FR-011,FR-013 | US3.AC6 / SC-002–006 | P1–P5 predeclaredmethod/U/limits/source hashes and final matrix; verification/hardware.md,acceptance.md; unknowns blocked, no inferredpass |

Кожна фактична перевірка фіксує команду/config/version/hash, очікуване/факт/стан і непроведені cases. Програмні тести, target build і фізична дія не підміняють один одного. Test-first не обов'язковий. Brownout/watchdog повне прийняття05/06, pump direction/power/current/thermal/independent cutoff06; тут лише задокументований boot policyD59 і no-load startup/reset/upload.

## Структура проєкту

### Документація feature

Створено plan.md, research.md, data-model.md, contracts/pump-output.md, quickstart.md. Spec синхронізовано з явним D59 і технічним D60; початковий checklist залишається quality review, не physical pass. Tasks — окремий запит; verification створюється при фактичних виконаннях.

### Наявні та заплановані файли реалізації

~~~text
firmware/components/bridge_core/
  include/bridge/pump_output.hpp     # нові portable DTO/OutputPolicy/port
  pump_output.cpp                   # нова finite policy/percent encoding
firmware/components/platform_esp32/
  include/platform/pump_output.hpp  # новий єдиний EspPumpOutput
  pump_output.cpp                   # новий LEDC/GPIO adapter
  gpio_safe_output.cpp              # зберегти profile check; прибрати дубльований EspSafeOutput writer
  include/platform/safe_output.hpp  # profile boundary; старий writer замінює єдиний adapter
  startup_policy.cpp                # startup failure context/rc04 formatter
  include/platform/startup_port.hpp # snapshot/ownership metadata
  rc_capture.cpp, include/platform/rc_capture.hpp # output service у serialized owner; accuracy gating
  local_diagnostics.cpp, include/platform/local_diagnostics.hpp # coherent snapshot/archive solewriter
firmware/main/app_main.cpp          # композиція/handoff/config; без controlcommands
firmware/main/Kconfig.projbuild     # default-off bench choice/trace-step
firmware/sdkconfig.pump04-{waveform,stress,accuracy}.defaults # нові окремі fragments
firmware/components/*/CMakeLists.txt # sources/native LEDC dependency; core C++11
scripts/build.py                   # target-only04 flags/reject incompatible options/build identity
scripts/verify/config.py           # require-pump04 variant checks
scripts/verify/pump_output_trace.py # новий offline schema/timing/duty/voltage-reference validator
 tests/host/pump_output_{policy,adapter,evidence}_tests.cpp # meaningful new fixtures
 tests/host/pump_output_header_probe.cpp                 # strict C++11/no MCU
 tests/tooling/test_pump_output_trace.py                 # malformed/boundary/offline evidence
 tests/tooling/test_build_contract.py                    #04variant guards/regression
~~~

Шляхи нових файлів заплановані, не створені в Plan. Справжні02ControlCore, capture03/RMT/D56, board JSON і pins зберігаються. Baseline default fragment не вмикає bench; спеціальні fragments іmanifest відрізняють небезпечний для підключеного насоса bench image від normal.

## Компроміси

| Вибір | Причина | Відхилена альтернатива |
|---|---|---|
| D59 boot-based bench | Явна відповідь користувача після доказу reset alias; без нового input/GPIO | Power-cycle discriminator без незалежного апаратного джерела недоведений |
|13bit/APB/stop idle endpoints | Точний divider, мала quantization, явні staticLOW/HIGH | Full-scale overflow count/fade/паралельні writers зайві |
| Один existing owner | Fault-before-output, збережений порядок core/capture | Другий task із stale snapshots створює race/rearm ризик |
| Bench-only bounded evidence | Фізичні timing/capture evidence без per-edge USB та нового GPIO | Console arrival times не доводять deadline, необмежена історія порушує03 |
| Окремі waveform/stress/accuracy images | Скінченна послідовність і різні evidence scope | Freeze128records не доводить600сloss/повну sequence |

## Уточнення checklist-guided перед Tasks

Контракт уточнює Fault для normal, serial ordering також для staticHIGH, незалежний від recorder LOW при overflow та failure path перед accuracy freeze. У моделі задані стабільні wire enum-коди, per-operation sdk_status/header summary та явний trace-step без default. Physical metadata має initial levels/bounds/gaps, а112мс означає початок сталого terminalLOW із10с спостереженням за наявним no-repeat guard, не звичайну LOW-частину PWM. Це уточнення прийнятих властивостей, не зміна режиму роботи чи фізичний pass.
