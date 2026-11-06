# План реалізації: прозора трансляція та Test/Fly05

**Ітерація**: `05-failsafe-arming` | **Дата**:2026-11-06 | **Spec**: [spec.md](spec.md)\
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming`\
**Гілка**: `05-failsafe-arming`, без створення/перемикання у Plan.\
**Стан**: Phase0/Phase1 завершені як дизайн D69–D73. G05 закрито конституцієюv2.0.0/D74 від2026-11-06; physical Q02/Q03/Q06 відкриті. Tasks/Implement не запускалися, нових firmware test/build/physical результатів немає.

## Короткий опис

Розширити наявний ControlCore02 та серіалізований capture/control/output owner03–04. Ordinary operational build — Test; Fly обирається явно. До першої повністю valid поточного boot LOW; потім команда, включно з високою, без arming/zero-handshake/додаткових 10 кадрів. Test утримує last-valid до age≥500 мс, далі LOW; загальна фізична межа від останнього accepted end —1 с за D66. Fly утримує last-valid незалежно від відсутності входу; нова trustworthy valid після resync автоматично відновлює трансляцію в обох режимах.

Q05 technical design делегований D66–D68. Обрано checked output recovery з 3 додатковими reinit спробами/100 мс spacing і post-recovery fresh-valid fence; fatal clock/protocol/LowUnconfirmed ведуть до reset через watchdog. Runtime TWDT100 мс із прямим ROM-reset hook не покладається на стандартний panic path із 10 с fallback. [Research](research.md) містить первинні джерела, rationale й альтернативи; [контракт](contracts/operational-control.md) — точні переходи та M05. Жоден вибір не є виміряною гарантією.

Насос дозує пальне/керує тягою; джерело — ArduPilot/Matek H743-Wing. Конвертор не керує запуском/горінням/відновленням двигуна. DIR постійно GND на MD20A, без MCU DIR. Всі operator tests05 без насоса; LOW означає brake, не power isolation/нуль витрати. Прийняття 06 потребує accepted05 й перевіреного силового стенда.

## Технічний контекст

| Пункт | Обраний дизайн / межа |
|---|---|
| Мова | Strict C++11 portable core без IDF/RTOS/GPIO/I/O/heap; app/platform C++17, no RTTI/exceptions як 01–04 |
| SDK/tools | Native ESP-IDF v5.5.6 commit880a203ae603d08a94d06aa01e0d6761958e5e9e; GCC14.2.0 esp-14.2.0_20260121; Python3.11.7/CMake3.30.2/Ninja1.12.1; host Apple clang21.0.0/CLTools26.6.0.0.1781586589/macOS SDK26.5. Хеші/шляхи у tools/environment/environment.lock.json, environment gate до reuse |
| Залежності | Наявні gpio/RMT/LEDC/esp_timer/USB/FreeRTOS та esp_system TWDT/esp_rom. Без SDK patches, сторонніх бібліотек, Arduino, radio/network/NVS/OTA feature |
| Профіль | c3-supermini-pinout-01/unit c3-supermini-188b0e93032c; GPIO3 input,GPIO0 output,3V3/GND; DIR–GND hardware. Профіль software-design approved, не bench-ready |
| Захоплення | Чинні Q04:50 Гц,width900..2100 мкс,period18000..22000 мкс; RMT1 МГц/filter0/stop5 мс/48symbols; queues64edges/8batches; rearm≤1 мс/processing≤10 мс; stuck100+10 мс; guard12+10 мс, D56 floating pulls |
| Вихід | LEDC low-speed timer0/channel0/APB80 МГц/13bit,1000 Гц,static stopIdle0/1,no fade; той самий GPIO0 owner |
| Час/нагляд | Checked signed64 raw→uint64 мкс, accepted-end окремо від service-time; T500 мс, age≥T; TWDT100 мс runtime task subscription; IWDT500 мс; [M05](contracts/operational-control.md) визначений до реалізації |
| Storage/RAM | Без persisted duty/mode/історії. Додаткова portable operational state≤2048B; SDK init allocations окремо. Bench-only extra RAM≤12288B; реальні map/stack high-water перевірити пізніше |
| Diagnostics | Новий rc05 v1 консервативна межа≤347B/384B padded, одна latest snapshot/один committed record, spacing≥100 мс,zero-wait/drop; no command source |
| Масштаб | Один локальний вхід/вихід.07–09, двигун і силове прийняття 06 виключені |
| Поточні gates | Q05 technical selections зроблено D69–D73; G05 закрито D74; Q02/H02/H03/Q03/Q06 та equipment/U/reset anchor до відповідних physical steps |

[Джерела й актуальні acceptance01–04](spec.md) прочитані разом зі спільною основою та prerequisite contracts. Застарілий prose про «не реалізовано» не перезапускає completed software; host/build не закривають open physical tasks. Локальний SDK source доступний для research; доступність усіх pinned tools та можливість збірки цим Plan не підтверджені.

## Перевірка конституції

Наведена таблиця зберігає історичні pre-research/post-Phase1 оцінки за конституцієюv1.0.0, зроблені до окремо ухваленої поправки. Тоді D64/D65 були explicit overrides, а G05 блокував implementation. Це не переписано як нібито старий принципII вже відповідав політиці. Поточна конституція —v2.0.0/D74: G05 закрито; актуальний review після поправки наведено одразу після історичної таблиці. D10 Specify-only тепер норма принципуV, Plan не змінює гілку.

| Gate | До research | Після Phase1 |
|---|---|---|
| I — Scope | pass: один C3/MD20A channel, fixed mapping/1000 Гц/radios off, R01/R04/R06/R10 | pass: D63–D65; no pump/engine/07–09, fixed DIR/profile |
| II — Safety | pass для прямо запитаного Plan за D64–D68 overrides; implementation blocked G05, physical blocked Q02/Q03/Q06 | pass для scoped design: Test500 мс/1 с, Flyhold/new-valid recovery, checked failure/watchdog, bias/power cutoff separate. **Старий принцип II конфліктує; G05 implementation blocked**, physical suitability не прийнята |
| III — Architecture | pass: portable existing core/profile/native IDF/sole owner | pass: core config extension, separate ordering/health/history, single writer/handoff, no unsupported platform |
| IV — Evidence | pass: pinned lock, inherited evidence categories, delegated numeric selection | pass для design: M05 declared; H/B/P traceability й окремі planned records, primary SDK research. Hardware/results not_run; physical gates blocked |
| V — Workflow | pass: only Plan, explicitdirectory05, setup-plan returned05, preserve work; D10 Specify-only | pass: read-only research agents за skill, docs05+пов'язані decision/spec updates; no branch/commit/push/device/flash/load/tasks/implementation. Hooks відсутні |
| VI — Decisions | pass: D63–D68 settled, Q04 не defaults, G05/physical owners явні | pass: D69–D73 закривають delegated technical choices; formal governance та physical inputs залишають owner/deadline, no measured claims |

### Поточний review після поправкиv2.0.0/D74

Ця перевірка узгодженості є частиною Constitution synchronization, не повторним запуском Plan/research або реалізацією. Поточна редакція принципів усуває governance-конфлікт; старі записи таблиці вище зберігають дату й історичний зміст.

| Gate | Поточний стан документації після D74 |
|---|---|
| I — Scope | pass: один C3/MD20A канал, D63 converter-only, fixed map/1000Гц, no radio/07–09 |
| II — Safety | pass для дизайну: Test/Fly/startup/recovery/current-vs-history/technical failure/watchdog відповідаютьv2.0.0; G05 закрито, physical suitability blocked Q02/Q03/Q06 |
| III — Architecture | pass для дизайну: existing portable core/sole owner/adapters/profile, no unsupported platform claim |
| IV — Evidence | pass для документації: D69–D73 таH/B/P traceability збережено; actual firmware/host/build/physical05 not_run |
| V — Workflow | pass: лише прямо дозволена Constitution synchronization; branch/commit/push/device/flash/load таінші фази не виконані, D10 не застосовано дляbranchmutation |
| VI — Decisions | pass: D74 формально усунув суперечність, technical D66–D73 не переобрані; physical owners/deadlines відкриті |

G05 closure не є constitutional waiver, дозволом автоматичного Implement або physical acceptance. За нової суперечності або неможливості довести M05 залежна робота/прийняття зупиняються, межі не послаблюються.

## Архітектура, дані та порядок роботи

```text
GPIO3 -> RMT width + GPIO ISR estimated edges -> bounded queues/ingress flags
                                                   |
1ms notify timer -> existing rc-control owner -> CapturePolicy + one ControlCore
                                                   |  validation/barrier/commit
                                                   |  evaluate Test/Fly/technical health
                                                   v
                                    one EspPumpOutput -> GPIO0/LEDC -> MD20A
                                                   |
                                coherent latest snapshot -> sole USB writer
owner completed checkpoint -> task WDT feed; timeout -> ROM reset hook
```

1. Constructors/config не створюють nonzero. Foundation перевіряє profile й primedLOW, setup/retry D15 (5 спроб,≥1 с від completion) лишається до handoff. Його ReadyDisabled означає лише готовність основи, не команду. Hardware захищає pre-firmware інтервал.
2. Для operational створити persistent owner і notify timer, runtime TWDT/add(owner) checked; до supervision readiness nonzero заборонений. Одноразовий handoff передає GPIO0/core тому самому owner. App більше не poll-ить startup/output. Worker/monitor не видаляються через RMT/GPIO cleanup; ingress retry тільки owner.
3. Startup GPIOLOW readiness окремо від timer/channel/route readiness. Ініціалізувати LEDC з duty0, checked low path. Почати нове capture спостереження після outputReady; clean first fully-valid може бути high без додаткового gate. Startup failure history не є permanent operational lock післяуспіху; finite04 зберігає старий startup_had_failure gate.
4. ISR тільки ingress/epoch/seq/estimated time/flags; notify без друку. Owner знімає pending losses, бар'єри, bounded drain64/8, rearm/recovery. Timer лише notify. Complete candidate проходить association/period/width і processing budget до commit; pending faults повторно знімаються перед commit/apply. Непридатний sample не отримує new valid і не підміняє end.
5. Operational core зберігає окремі accepted-end та shared-now ordering. Input status/history не викликає legacy fault(now) у measurementstream. Legacy default constructor/API лишають 02 fixtures незмінними; новий config проходить через CapturePolicy, другогоядра немає.
6. Effective priority: Fatal/LowUnconfirmed →LOW/ResetPending; technicalretry/exhausted/fence →LOW; no current-bootvalid →LOW; Testage≥500 мс →LOW; інакше last valid. Fly input loss неінгібує. Новий acceptedvalid автоматично recovery-high; invalid/activity/reconnect некоманда.
7. Перед/після кожного SDK nonzero, включно зі staticHIGH, owner знімає technical health/pending ingress та reevaluates mode; між set/update recheck. Старий requestepoch/generation не повертає output після LOW. Lock не тримається під SDK; фізичнаатомарність ISR/API не обіцяється. Точний порядок гонок — контракт§2.
8. Output errors ведуть у checked LOW/fallback/stagedreinit. D75:3 додаткові спроби на епізод, кожний reinit start витрачає одну; лише checked застосування нової post-fence valid-команди, включно з valid0%, скидає count. Reinit0 сам бюджет не поновлює; подальша apply failure використовує залишок, а за count3 переходить у ExhaustedLow. Успішний третій reinit ще може завершити епізод успішним apply. Інтервал≥100 мс від completion невдалої операції, успішний reinit0 дає нову epoch/fence та LOW до valid end>recovered_at. ExhaustedconfirmedLOW живе без appautorestart; runtime clock/protocol/LOWunknown →no feed/reset за armed watchdog; watchdog init/add failure до handoff →SetupBlockedLOW без недоведеної reset guarantee, після handoff feed API error →явний emergency ROM reset після best-effortLOW. Не використовувати historical stickycapturebits як technicalfault.
9. Checkpoint після повного ingress/control/output сервісу; only subscribed owner feed. Прогрес не вимагає RC valid/USB reader. Test feed при nonzero лишедо T; на/після T після LOW. Frozen raw не продовжує feeding. SelectedROM hook обходить stock panic10 с; actual IRAM/log elimination/config/map перевіряютьсяпри implementation.
10. Diagnostics отримує копію latestsnapshot, форматує поза lock; поля calculated/effective/applied/history/current різні. One committed-record reconnect caveat збережено. Bench archives/injection відділені compile-time, production безісторії. Reset стирає heldduty, не поновлює старийкрок чикоманду.

## Обрані технічні межі та доказовість

Повний M05 — [контракт§4](contracts/operational-control.md). TestT500 мс залишає 500 мс запасу в userlimit1 с. Healthy deadline→checked LOW≤2 мс; new-valid end→checkedAPI≤19 мс/physicalstate≤20 мс. TWDT runtime100 мс; nominal no-ISR stages100+200=300 мс, але standardpanic може зруйнувати цей backstop. Саме тому вибраний ROM hook, IWDT500 мс і audit до nonzero acceptance.

Для заявлених stallcases встановлено lastqualified feed→reset≤350 мс та reset→physical LOW≤100 мс; разом T500+350+100=950 мс. Це engineering allocations, не source timing guarantee. Прямий фізичний 1 с залишається незалежним обов'язковимкритерієм, latencyU≤1 мс й L+U≤limit. Якщо actual reset не має незалежного anchor або немає waveform/bias доказу, відповідні partialmetrics blocked; USB timestamps не підставляються. Довільна відмова ROM/panic/WDT/clock/pad не оголошується покритою. Watchdog не силоваізоляція.

Довгий Flyloss:≥600 фізичних секунд у 0/80/100%,reader active/slow/absent, потім directnew high/new low. Здоровий capture окремо проходить D53–D55 короткі 128RAMwindows і 600 с zero-loss/timing з maxdiagnostics; exact boundary/malformed fixtures host. У summaryreuse601MCU секунда+lowerbound600phys за D62 без finiteoutputend. M04 frequency/duty/voltage/stableendpoint/1 мкс detection не послаблюються. P3 відхиляє prematureLOW при age<T: upper-bound1с недостатньо; independent age/waveform correlation і uncertainty задаються до run, інакше hold-part blocked. Exact T±1 fixtures — host, без нового physical tolerance. Спільна міграція latch/observer виконується T013 до US2 checkpoint; T028 лише інтегрує Fly з уже підготовленим шляхом. Змінадокумента сама не запускає повтор unrelatedhardwaretests.

Timing evidence D76: один compile-time bench-сценарій, accepted anchor оновлюється в RAM; журнал≤64 records зберігає значущі transitions/пов’язані API brackets/all failures/retry/D75 closure, не повторні успіхи без зміни стану. Start до першої traced operation, freeze після declared final event/checked result; missing anchor/result або overflow→incomplete без впливу на control. Точні bounded layouts/codes/digest визначаються в T022 перед collector і T046/T047 fixtures, T050 використовує ту саму схему. Це уточнення прийнято власником після Plan, не повторний Plan/research і не зміна M05/accuracy/stress budgets.

## Збірки та сумісність

Плановані wrapper inputs: target-only `--operational-mode test|fly`; за відсутності явного legacy/bench ordinary —Test. `--legacy-disabled` дає явний compatibility image LOW/disarmed; наявні `--rc03-bench`/`--pump04-bench` обирають legacy03/finite04. Explicitoperational mode разоміз legacy/finite flags відхиляється до configure. Нові `--pump05-evidence timing|stall|accuracy|stress` тільки explicitno-pump instrumentation operational family; негенеруютьфіксовані 04duties й невмикаються default.

Kconfig family/operational choice визначає composition; звичайний direct native build теж defaultTest. Base defaults підтримують operationalwatchdog settings; legacy-specific fragment відновлює 04 watchdog/config choices і не включає ROM hook. Existing04fragments/grammars/finite contracts зберігаються, SDK/config/raw evidence попередніх commit не переписуються. Actuallegacybehavior/buildregression треба довести заново післяреалізації, не claim binaryidentity.

Manifest: family,mode,policyversion,M05constants,evidencepurpose,source hashes,SDK/profile/config hashes. Verifier відхиляє wrong effective mode,unexpectedlegacy/bench,missingROM hook/WDTruntimeconfig,radio/dependency/fade/logging changes. Підтвердження ordinarydefaultTest потребує clean configure з відсутнім modeflag, explicitTest і explicitFly окремо; назва fragment не proof.

### Перевірки й простежуваність

H/B/P — сценарії validation, не tasks.md. Усі результати 05 not_run; майбутні actual records — verification/host.md,build.md,config-review.md,hardware.md,acceptance.md ізкомандами/config/source hashes/expected-actual/status.

| R / FR | US / SC | Запланована перевірка |
|---|---|---|
| R01,R03,R10 /FR-001 | US1.AC4,US5.AC4 /SC-001,002,008 | H1 core/headerisolation; B1 nativegraph/profile/noDIR/radiosoff |
| R07,R11 /FR-002 | US1.AC4 /SC-002 | B2 default/explicitTest/Fly/legacy/finite04,negativeflags,manifest/effectiveconfig |
| R07,R08 /FR-003 | US1,US3.AC3 /SC-002 | H2 firstvalidhigh/nozero/reset/noheldprior; P1 startup/reset/bootloader/upload/brownout/watchdog |
| R04,R05,R06 /FR-004 | US1.AC2,US2.AC2,US3.AC1–2 /SC-001,007 | H1 map/rounding/width/period/invalid; P2 waveform/endpoints/M04voltage |
| R05,R07,R09 /FR-005,006 | US2 /SC-001,003 | H3 T499999/500000/500001,1 с−/0/+,late/old/fault-now/order/races; P3 directTestend→LOW≤1 с/normal≤20 мс |
| R07,R09 /FR-007 | US3.AC1,3–4 /SC-004,007 | H4 Flylongage>UINT32MAX/0/80/100/stuck; P4 ≥600physloss perreader,hold/recoverhigh/low |
| R05,R07,R08 /FR-008 | US2.AC3–5,US3.AC2 /SC-001,004 | H4 epoch/barrier/tworising/processingcommit/noold; P3/P4 recovery high |
| R05,R07,R09,R11 /FR-009 | US2.AC2,US3.AC1–2,US5.AC1 /SC-004,006 | H5 inputfault≠technical/ownerlifetime/endedstuck/masking; P5 inheritedcapture100+10/rearm/processing |
| R06,R08,R09,R11 /FR-010 | US4.AC1–2 /SC-005 | H6 eachAPI/fallback/cleanupfail/retry3/100ms/fence/stale/exhaustion/D75 episode count (reinit-success/apply-failure, third-success/valid0, closure/new-episode); P6 technicalresponse/routevoltage |
| R05,R08,R11 /FR-011 | US4.AC3 /SC-005 | H6 clockrawnegative/frozen/regressive/overflow/oldsample,independenttimeorder; P6 selectedfaultinjection |
| R08,R09,R11 /FR-012 | US1.AC3,US4.AC4–5 /SC-002,005 | H7 qualified feed/nonfeed/ordinarynofaultFly; B3 WDT/hook/IRAM/logs/ROM; P7 block/busyloop/IRQstall→reset/LOW/direct1 с |
| R09,R11 /FR-013 | US5.AC1–3 /SC-006 | H8 rc05bound347/384/versionack/drop/reconnect; P5 maxpayload/active,slow,absentreader |
| R01,R08,R12 /FR-014 | US1.AC3,US2,US3,US4.AC4,US5.AC4 /SC-003–008 | P1–P7 operatorgates/no-pump/sourceinterface; receiverbeforeuse,notHJblocker |
| R04,R05,R06,R09,R12 /FR-015 | US2.AC1,US3,US5.AC3–4 /SC-003–008 | B4 benchRAM/stack; P2–P7 declaredlimits/U/anchors/D53–D55/M04/D62 |
| R12 /FR-016 | US5.AC4 /SC-008 | H1–H8 meaningfulnegative/boundary/faultfixtures; B1–B4 pinnedbuilds; actual recordsH/B/Pseparate |
| R07,R08,R12 /FR-017 | US5.AC4 /SC-008 | Review G05/D10/useroverrides/decisions; implementation gate, noautomaticphase |

Target matrix післяреалізації: ordinarydefaultTest,explicitTest,explicitFly,explicitLegacyDisabled; existing03diagnostics/accuracy й 04waveform/stress/accuracy0..5; targeted05evidence кожного mode,negativecombinedflags. Збережені 22/28 suites не означають fixed майбутній count; meaningful newtests + legacyregression і negativeprobe потрібні. Config/compile/ELF audit окремийвід buildpass. Немає physical pass закомпіляцією.

## Структура проєкту

### Артефакти feature

```text
specs/05-failsafe-arming/
  spec.md                       # прийнята spec + технічне доповнення Plan
  plan.md                       # цей дизайн
  research.md                   # Phase0, первинні джерела й рішення
  data-model.md                 # DTO/state/time/lifetime/evidence
  contracts/operational-control.md # embedded межі, M05, rc05 grammar
  quickstart.md                 # майбутні validation команди/operator gates
  checklists/requirements.md    # наявний Specify checklist, не physical acceptance
```

Tasks.md не створюється. verification/ з actualresults створюється лише під фактичнимвиконанням, не якпорожній pass у Plan.

### Заплановані code boundaries — зараз не реалізовані

- Existing bridge_core/control.hpp,control.cpp: config/operational evaluate/history/fence бездругого core; rc_capture.hpp/.cpp: configpass-through, provisionalcandidate→commit, separateinputstatus/serviceordering; legacybranch unchanged.
- Existing bridge_core/pump_output.hpp/.cpp: DTO ролей/health/retry stages без зміни finiteOutputPolicy04. Карту не копіювати в новий competing reducer.
- Existing platform_esp32/rc_capture.hpp/.cpp: persistentowner/monitor vs ingressresources, operational service/observers, qualifiedcheckpoint, checkedraw leaves; pump_output.hpp/.cpp: roles/readiness/checkedstages/recoveryroute. Startup/diagnostics/app_main composition та one-time handoff.
- New platform_esp32/include/platform/control_watchdog.hpp + control_watchdog.cpp: tasksubscription/reset hook/checked supervision port; hostfake поза ESP_PLATFORM. Окремий tiny control_watchdog_reset.cpp без weak SDK prototype забезпечує strong hook; це доводить nm/map. Новий GPIOwriter не додається.
- Kconfig.projbuild,firmware/sdkconfig.defaults і явні legacy/operational/evidence fragments; wrapper scripts/build.py + verify/config.py: family/mode/manifest/negativeflags. ExistingCMake sources/esp_system/esp_romdependencies; buildsystem nativeIDF тойсамий.
- Tests/host operationalcore/owneradapter/recovery/watchdog/formatter fixtures, existinglegacyregressions; tests/tooling mode/config/evidenceparsers. Proposed scripts/verify/operational_trace.py тільки files, no serial/hardware/reset sideeffects.

## Компроміси та відкриті gates

| Вибір | Причина | Відхилена альтернатива |
|---|---|---|
| T500 мс у 1 с total | Залишає reset/bias запас; userdel егував T | T=1 с без reactionmargin; default100 мсбезрішення |
| Configuredexistingcore + soleowner | Зберігає map/validation/ordering й legacybaseline | Другий core/outputtask,modelclear як recovery |
| Technicalretry3/100 мс+freshfence | Автоматичнеобмеженевідновленнябез stalereplay | Blanketbootterminal для input loss або cachedhighreplay |
| EmergencyROMresetWDT hook | Stockpanic може 10 с; noSDKpatch/noGPIOwriter | Сподіванняна silentpanic як hard1 с proof; прямеперепрограмування MWDT/HAL |
| rc05 окремийграматичний version | Нові current/history/mode поля в 384B | Додаваннядополного 383Brc04,обрізанняполів |
| Explicitbenchinstrumentation | Directphysicaltiming/faultproof без productionhistory | USBtimestamp/pass з API/постійний archive |

G05 закрито D74/конституцієюv2.0.0 та synchronization2026-11-06 за окремим прямим завданням; ця поправка не змінює technical design і не запускає implementation. Q02/H02/H03:01power/bias/reset/прилад; Q03:inputlevels/conditioning/timing/actualMatek/receiverbeforeuse; Q06:04driverinterface/jumper/no-pump та 06powerpath/emergencycutoff/load. Для watchdog budget додатковопотрібний незалежний resetanchor/verifiedreset→LOW; якщо його немає, partialmetricblocked навітьза validdirectend→LOW. Немаєнепогоджених softwaredefaults, але прийняття hardware та implementationpermission цимдизайномнестворюються.
