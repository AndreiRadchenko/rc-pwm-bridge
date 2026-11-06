# Чекліст якості вимог: operational межі05

**Мета**: reviewer-огляд часових меж, відновлення, доказів і сумісності перед Tasks.\
**Створено**: 2026-11-06\
**Feature**: [spec.md](../spec.md)\
**Глибина**: reviewer; аудиторія — власник продукту та reviewer перед `/speckit-tasks`.\
**Походження**: `speckit-checklist` усередині явно викликаного `speckit-checklist-guided`; це перевірка написаних вимог, не виконання firmware tests.

Аргумент базового workflow: `operational-boundaries — reviewer depth; audience: owner/reviewer before Tasks; focus: timing/freshness; recovery/supervision; evidence; compatibility/governance; Ukrainian; must-have: all six constitutional gates, D63–D74, inherited Q04 and physical evidence boundaries`.

## Повнота й межі

- [x] CHK001 Чи визначено один канал, converter-only відповідальність, постійний DIR–GND і виключення двигуна/насоса06/07–09? [Повнота, Spec §Обсяг і межі рішень]
- [x] CHK002 Чи відрізнено ordinary/default Test, explicit Test/Fly та незмінні legacy/finite04 images без runtime mode? [Ясність, Spec §FR-002]
- [x] CHK003 Чи визначені no-prior-valid, перша fully-valid high та втрата held duty після кожного reset без zero-handshake? [Покриття, Spec §FR-003]
- [x] CHK004 Чи узгоджено терміни build family, core configuration та output role без включення finite04 у operational core mode? [Узгодженість, Ambiguity, data-model §Незмінна конфігурація]

## Валідація та часові межі

- [x] CHK005 Чи однозначно задані validation-before-clamp, ширина/період, rounding і invalid hold без refresh? [Ясність, Spec §FR-004]
- [x] CHK006 Чи визначено accepted pulse_end_us як єдине джерело freshness з урахуванням processing/output latency? [Повнота, Spec §FR-005]
- [x] CHK007 Чи записані T, включна межа age≥T і окремі T−1/T/T+1 критерії без підміни capture100+10мс? [Вимірюваність, Spec §FR-006, §SC-003]
- [x] CHK008 Чи розрізнено загальну фізичну межу1с, внутрішні API budgets та умовну watchdog budget allocation? [Ясність, Spec §SC-003, contracts §4]
- [x] CHK009 Чи поточні FR/SC і матриця простежуваності відображають обрані D69–D73 замість застарілих design-pending позначок? [Узгодженість, Conflict, Spec §Технічне доповнення Plan]

## Відновлення й гонки

- [x] CHK010 Чи явно відокремлені current capture status, sticky history та technical inhibition зі збереженням legacy baseline? [Узгодженість, Spec §FR-009]
- [x] CHK011 Чи визначені автоматичні recovery-high/valid0 після resync, без нового arming або manual clear? [Покриття, Spec §FR-008]
- [x] CHK012 Чи описані old/duplicate/pre-barrier/overflow samples, two-rising resync та commit тільки після processing check? [Граничні випадки, Spec §FR-005, §FR-008]
- [x] CHK013 Чи визначено порядок одночасних deadline/new-valid/fault/apply та відхилення stale generation? [Ясність, Spec §FR-010, contracts §2]
- [x] CHK014 Чи охоплює Fly hold відсутність/invalid/stuck input,0/100% і великий age без прихованого cutoff? [Покриття, Spec §FR-007, §SC-004]

## Технічні відмови й нагляд

- [x] CHK015 Чи визначені checked LOW/fallback, partial setup/cleanup ownership і LowUnconfirmed окремо від physical LOW? [Повнота, Spec §FR-010, contracts §3]
- [x] CHK016 Чи обмежені кількість/інтервал/тривалість retry, exhaustion та fresh-command fence без cached-high replay? [Вимірюваність, Spec §FR-010, contracts §3–4]
- [x] CHK017 Чи однозначно визначено завершення technical failure episode та момент поновлення бюджету трьох reinit спроб? [Ambiguity, Gap, data-model §ControlSnapshot05]
- [x] CHK018 Чи відрізнено старий measurement від несправного shared clock і задано negative/regression/freeze/overflow/long-uptime семантику? [Граничні випадки, Spec §FR-011]
- [x] CHK019 Чи визначено єдиного owner, збереження його lifetime під час ingress retry та контроль фактичного повного progress? [Повнота, Spec §FR-010, §FR-012]
- [x] CHK020 Чи розрізнено watchdog init/add до armed subscription і feed failure після handoff, без недоведеної reset guarantee? [Винятки, Spec §FR-012, contracts §3, §5]
- [x] CHK021 Чи явно задані watchdog coverage, reset path, startup LOW, обмеження panic/ROM/oscillator і відсутність гарантії силової ізоляції? [Ясність, Spec §FR-012, research §RCH-04]

## Якість критеріїв і доказів

- [x] CHK022 Чи відокремлені host, build/config/ELF та фізичні результати з honest not_run/blocked статусами? [Узгодженість, Spec §FR-016, §SC-008]
- [x] CHK023 Чи задані незалежний reset anchor, accepted-pulse provenance, uncertainty та заборона USB timestamps як фізичного часу? [Вимірюваність, Spec §FR-015, quickstart §План операторських сценаріїв без насоса]
- [x] CHK024 Чи записані gapless/static-endpoint/cutoff observation criteria й окремі правила latency L+U та успадкованих D54 E/U? [Вимірюваність, Spec §SC-006–007, quickstart §План операторських сценаріїв без насоса]
- [x] CHK025 Чи розділено long Fly loss, healthy capture600с, короткі accuracy windows і finite04 сценарії? [Узгодженість, Spec §SC-004, §SC-006]
- [x] CHK026 Чи розрізнено консервативний sizing payload, максимальний transport frame та фактичний formatter/build evidence? [Ясність, Spec §FR-013, contracts §6]
- [x] CHK027 Чи визначено вимоги до immutable bench export identity/count/digest, відхилення mixed/incomplete/overflow і втрати RAM при reset? [Повнота, Spec §FR-015, data-model §Bench-only evidence]

## Нефункціональні вимоги й залежності

- [x] CHK028 Чи задані latest-only/≤10записів/с/zero-wait/one-committed reconnect caveat без другого writer або command input? [Повнота, Spec §FR-013]
- [x] CHK029 Чи встановлено bounded RAM/resources і поведінку evidence/statistics overflow без operational Fly cutoff? [Граничні випадки, Spec §FR-007, data-model §Нагляд і ресурси]
- [x] CHK030 Чи збережено власників і строки фізичних Q02/Q03/Q06 та receiver/radio-loss до його використання без блокування HJ-only? [Залежності, Assumption, Spec §Відкриті залежності та строки їх закриття]
- [x] CHK031 Чи поточна конституціяv2.0.0/D74 і всі шість gates узгоджені зі збереженою історією v1/D10/G05? [Узгодженість, Spec §FR-017, plan §Перевірка конституції]
- [x] CHK032 Чи вимоги R01,R03–R12 простежуються до FR/US/SC/H/B/P, а посилання на успадковані platform/core boundaries називають наявні шляхи? [Простежуваність, Spec §FR-016, research §RCH-03]

## Примітки

- Прапорець означає, що вимога має достатній текстовий доказ або погоджене виправлення; він не означає physical/firmware pass.
- Prerequisite workflow повернув `FEATURE_DIR=/Users/mac/Projects/Scopa/rc-pwm-bridge/specs/05-failsafe-arming`; усі generated artifacts цього огляду належать05.
- `.specify/extensions.yml` відсутній: before/after checklist hooks не зареєстровані.

## Triage за фактичним текстом

A — уже визначено; B — виправлено очевидну неузгодженість; C — потрібне рішення власника.
Шляхи в таблиці відносні до `specs/05-failsafe-arming/`; номери рядків — стан до відповіді C.

| Пункт | Група | Прочитаний текст / місце |
|---|---|---|
| CHK001 | A | `spec.md:13` — «Мета05 — завершити прозоре перетворення одного перевіреного RC-PWM сигналу на PWM MD20A, режимну поведінку Test/Fly та автоматичне відновлення перед к…» |
| CHK002 | A | `spec.md:156` — «- **FR-002 (R07,R11)**: Окремі immutable build configurations Test/Fly; ordinary build — Test, Fly явно; build identity та diagnostics однозначні. Run…» |
| CHK003 | A | `spec.md:157` — «- **FR-003 (R07,R08)**: Кожний boot/reset починає без prior-valid/held duty з LOW; перша fully-valid поточного boot автоматично транслюється, включно …» |
| CHK004 | B | `data-model.md:7` — «`ControlConfig`: core mode=`LegacyDisabled\|Test\|Fly`; policy_version=1; Test timeout_us=500000, valid_apply_limit_us=19000, physical_limit_us=100000…» |
| CHK005 | A | `spec.md:36` — «1. Width900–2100 мкс і period18000–22000 мкс включно; підтримка входу лише50 Гц. Period між послідовними trustworthy rising; unknown period не є valid…» |
| CHK006 | A | `spec.md:159` — «- **FR-005 (R05,R07,R09)**: Freshness походить від accepted pulse_end_us у shared monotonic domain з урахуванням processing/output latency; old/pre-ba…» |
| CHK007 | A | `spec.md:239` — «- **D69**: Test T=500000мкс, age≥T; межі499999/500000/500001мкс. Загальна physical last-valid-end→LOW≤1с незмінна. Нормальний end→checkedAPI≤19мс/dire…» |
| CHK008 | A | `contracts/operational-control.md:61` — «Алгебра stalled Test: checkpoint за nonzero тільки при age<T; на/після T feed тількипісля checked LOW. Тому найпізніший nonzero feed≤t_v+500 мс;350+10…» |
| CHK009 | B | `spec.md:160` — «- **FR-006 (R07,R08,R09)**: Test hold до age<T, при age≥T вимагає сталий LOW; за відсутності нових valid загальний last-valid-end→physicalLOW≤1000000 …» |
| CHK010 | A | `contracts/operational-control.md:21` — «У platform output_faults/output_service для operational прибрати blanket OR усіх sticky capture bits у terminal04. Перед кожною nonzero SDK operation,…» |
| CHK011 | A | `contracts/operational-control.md:30` — «6. Нова fully-valid після потрібного resync → Active/calc duty/permissiontrue, включно з high та 0%. Sticky history не забороняє перехід; resync сам н…» |
| CHK012 | A | `data-model.md:23` — «Обробка candidate двофазна: parse/association/width/period → завершення processing-budget check → commit valid. До commit немає update freshness/calcu…» |
| CHK013 | A | `contracts/operational-control.md:32` — «Одночасні події: owner спочатку consumes pending fault/barrier, commit придатних нових candidates, потім evaluate з fresh now перед кожним hardware ap…» |
| CHK014 | A | `spec.md:161` — «- **FR-007 (R07,R09)**: Fly за справних часу/виходу hold last-valid незалежно від тривалості input loss, включно з0/100%; signal/capture status/histor…» |
| CHK015 | A | `contracts/operational-control.md:40` — «На outputAPI failure: зафіксувати перший status/op/time-validity, inhibit nonzero/нова epoch, checkedstop0. Якщо channel unavailable або stopfailed — …» |
| CHK016 | A | `contracts/operational-control.md:42` — «Recoverable після confirmedLOW: до 3 додаткових reinit спроб, нераніше 100000 мкс від failure/завершення попередньоїневдачі. Кожна спроба — неблокуючи…» |
| CHK017 | C | `data-model.md:32` — «- current_technical_bits16/history_technical_bits16 (current очищується після успішного reinit, history лишається; AwaitFresh fence окремий), last_sdk…» |
| CHK018 | A | `data-model.md:47` — «Raw clock signed64 перевіряється до unsigned cast в усіх owner/ISR/SDK leaves. Негативне/регресія shared-now, неможлива арифметика → FatalClock, не пі…» |
| CHK019 | A | `data-model.md:53` — «ProgressCheckpoint: progress_seq64, trusted_raw64, core evaluated/request generation, output-known/LOW-status, checked watchdog feed outcome. Checkpoi…» |
| CHK020 | A | `contracts/operational-control.md:44` — «RMT/GPIO failures/rearm errors/overflow/stuck/malformed означають input unavailable: retry ingress≥100 мс з boundedresources, Testdeadline незмінний, …» |
| CHK021 | A | `research.md:49` — «**Межі**: довільна відмова panic/ROM/WDT/oscillator/pad не оголошується покритою. Для неможливого timing/відсутнього reset anchor — fail/blocked, без …» |
| CHK022 | A | `spec.md:198` — «Наведені verification шляхи плановані всередині05; результатів у Specify немає, файли evidence до виконання перевірок не створюються. H=host, B=target…» |
| CHK023 | A | `quickstart.md:66` — «Timing limits05: U_latency≤1 мс і L+U≤maximum. Це окреме правило для maximum latency; D54/M04 directE/U не змінюються. Не підбирати MCU/analyzer offse…» |
| CHK024 | A | `quickstart.md:64` — «Test cutoff — початок безперервного LOW у gapless записі, який триває≥100 мс без нової команди; після цього окремо recovery-high. Звичайна LOW частина…» |
| CHK025 | A | `data-model.md:61` — «Accuracy — чинні 128×48B+header≤256B=6400B без відбору добрих samples, frozen export після checked LOW/навмисної зупинки capture; такий explicit bench…» |
| CHK026 | B | `contracts/operational-control.md:88` — «Консервативна межа≤347B разом із framing,buffer385B із NUL,stresspadding до 384B перед newline. Timingmaxima й повний startupdiagnosticcontext лишають…» |
| CHK027 | A | `contracts/operational-control.md:90` — «Зберегти latest-only/spacing≥100 мс/≤10 записів/с/TX512/RX128/zero-waitdrain gate/onecommitted record/versionack/drop accounting03. Вже committed запи…» |
| CHK028 | A | `contracts/operational-control.md:90` — «Зберегти latest-only/spacing≥100 мс/≤10 записів/с/TX512/RX128/zero-waitdrain gate/onecommitted record/versionack/drop accounting03. Вже committed запи…» |
| CHK029 | A | `data-model.md:55` — «Production додаткова portable state/snapshot RAM≤2048B без існуючих queues/stacks/SDK contexts; це design upper bound для static assertions/map, не ac…» |
| CHK030 | A | `spec.md:69` — «\| Q02/H02/H03 /01 \| Реальні живлення/bias/startup waveform/прилад; USB+external power не прийняті \| До залежних підключень/живлення й фізичного при…» |
| CHK031 | A | `plan.md:47` — «### Поточний review після поправкиv2.0.0/D74…» |
| CHK032 | B | `research.md:27` — «**Локальні факти**: firmware/components/platform_esp32/pump_output.cpp:10–15,42–76 boot-terminal і bench_ одночасно керує LEDC init та permission. fir…» |

Початковий triage: A=27, B=4, C=1; до відповіді власника31/32 було закрито. Після D75 усі32/32 закрито; класифікація A/B/C збережена. B: composition terminology (CHK004), актуальні FR/SC/evidence status (CHK009), sizing upper bound (CHK026), platform/core source paths (CHK032).

## Resolution (2026-11-06, before /speckit-tasks)

### Рішення власника

| Пункт | Обраний варіант | Застосоване рішення |
|---|---|---|
| CHK017 / D75 | Рекомендований: після успішного застосування нової post-fence valid-команди, включно з valid0% | Кожний reinit start витрачає одну з3 спроб; самого reinit0 недостатньо. Count скидається лише після checked apply нової post-fence valid у її поточних epoch/generation. Apply failure до закриття використовує залишок; за count3 — ExhaustedLow після checkedLOW. Успішна третя reinit може чекати valid й завершитися успішним apply. Sticky history не стирається. |

Джерело — явна відповідь користувача2026-11-06; записано новим D75 у `docs/decisions.md` та C05-04 у spec. Синхронізовано spec/FR-010/SC-005, plan/H6, research/RCH-03, data-model/episode counters, contracts/§3–4 та quickstart/host/P6. Кількість3, spacing100мс і input-loss policy D64/D65 не переобрано. Watchdog/physical докази не додано.

### Очевидні виправлення B

| Пункт | Виправлення | Артефакти |
|---|---|---|
| CHK004 | Розділено build family, immutable core mode та output role; finite04 не є operational core mode; додано composition table | data-model.md, contracts/operational-control.md |
| CHK009 | Поточні FR/SC/traceability statuses посилаються на вже обрані D69–D73; датовану історію делегування збережено | spec.md |
| CHK026 | 347B названо консервативною верхньою межею розрахунку,384B — transport limit; це не результат formatter test | spec.md, plan.md, research.md, contracts/operational-control.md, quickstart.md |
| CHK032 | Уточнено наявні шляхи platform_esp32/bridge_core замість неіснуючого firmware/platform | research.md |

### Підтверджені текстом A

CHK001–003, CHK005–008, CHK010–016, CHK018–025, CHK027–031:27 пунктів; прочитаний текст і рядки наведені в triage table. Це підтвердження якості вимог, а не виконання відповідних перевірок firmware.

### Підсумок і межі

- 32/32 пунктів закрито: A27/B4/C1. Відкритих checklist рішень немає.
- Окремого analyze report у цьому workflow немає; Analyze/Tasks/Implement не викликалися, tasks.md не створено.
- Q02/Q03/Q06, придатність приладів/U/reset anchor, host/build/physical05 лишаються pending/not_run/blocked у своїх межах. G05 уже закрито D74, не цим чеклістом.
- Нові зміни огляду: цей checklist, шість feature05 artifacts і новий ADR D75; у metadata requirements.md замінено Markdown hard-break spaces на рівнозначний backslash для чистого whitespace check. Конституція, історичні ADR entries, specs01–04, firmware/tests не редагувалися для розв'язання пунктів.
- Commit/push у поточну `05-failsafe-arming` входять у явно викликаний `speckit-checklist-guided`, step6. Публікація пов'язаного пакета включає раніше створені в цій сесії артефакти05 та ухвалену constitution/template/input synchronization D74, необхідні для узгодженої основи; це не нова зміна governance під час checklist.
- Before/after hooks повторно перевіряються: `.specify/extensions.yml` відсутній; dispatch немає. Наступна окремо запитувана фаза — `/speckit-tasks`.

Фінальна перевірка документації:32 послідовні CHK-ID,32/32 позначено, traceability32/32, групи A27/B4/C1;51 локальне посилання feature05 має наявний target. D75 присутнє у всіх шести feature artifacts. Hash-порівняння зі станом до checklist підтвердило незмінність конституції, specs01–04, firmware/tests; старі ADR entries порівняно з Git HEAD і збережено. `git diff --check` без помилок. Це перевірка документів; firmware tests, target builds та фізичні перевірки05 не виконувалися.

## Синхронізація після guided analysis,2026-11-06

I1/C1/U1 розв'язано в окремо викликаному `speckit-analyze-guided`, без повторного запуску Checklist: спільна міграція input inhibition тепер T013 перед US2, P3 відхиляє prematureLOW окремо від upper-bound1с, D76 задає event journal/accepted anchor/compile-time start/freeze. CHK007–009,CHK013,CHK023–024,CHK027–029 уточнено відповідними spec/plan/contract/tasks, усі32 quality flags збережені. Початкові triage/Resolution вище є історією checklist; actual firmware/build/physical pass не додано. Подробиці й повторні метрики — [analysis-resolution.md](../analysis-resolution.md).
