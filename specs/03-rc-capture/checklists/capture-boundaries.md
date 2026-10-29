# Чекліст якості вимог: межі RC capture 03

**Мета**: reviewer-перевірка перед /speckit-tasks; capture/fault, часові межі, diagnostics та acceptance evidence.  
**Дата**: 2026-10-29  
**Feature**: [spec.md](../spec.md)  
**Контекст**: speckit-checklist-guided → speckit-checklist; depth reviewer, аудиторія — власник продукту/рев’юер. Попередній requirements.md збережено. Це перевірка написаних вимог, не виконання тестів.

## Повнота обсягу та контрактів

- [x] CHK001 Чи однозначно обмежено ітерацію одним RC-входом, ESP32-C3, RMT та відсутністю arming/load enabling? [Повнота, Spec §Scope and Decision Boundaries; FR-006]
- [x] CHK002 Чи розділені потрібна програмна основа 01/02 та відкриті фізичні gates? [Повнота, Spec §Scope and Decision Boundaries; FR-007]
- [x] CHK003 Чи узгоджено estimated GPIO timestamps, незалежну RMT width та заборону callback-time freshness? [Ясність, Spec §FR-001/002; Plan §Technical Context]
- [x] CHK004 Чи визначено достовірну відповідність measurement двом шляхам захоплення без евристики схожої width? [Ясність, Spec §FR-001; Contract §Асоціація та parser]
- [x] CHK005 Чи відрізняється очікувана зупинка на довгому HIGH від truncation із втратою даних у всіх вимогах? [Conflict, Spec §US2 scenario4; Plan §Архітектура крок5]
- [x] CHK006 Чи однакові включні межі width/period і правило validation-before-clamp? [Узгодженість, Spec §FR-002; SC-001]
- [x] CHK007 Чи визначено перший unknown period та period origin після invalid period? [Покриття, Spec §US1; D49]
- [x] CHK008 Чи відокремлено invalid period від незалежного ресурсного fault для unsupported/безперервного потоку? [Ambiguity, Spec §FR-002/003; Plan §Active guard]
- [x] CHK009 Чи відокремлено last-valid від довільної активності та збереження попереднього розрахунку? [Ясність, Spec §FR-002/006]
- [x] CHK010 Чи визначені пізні samples, fault barrier та часовий порядок подій ядра? [Повнота, Spec §US2; Contract §Policy → ControlCore02]
- [x] CHK011 Чи узгоджено fault latch, resync, valid під fault та незмінну заборону output? [Узгодженість, Spec §FR-006; SC-002]
- [x] CHK012 Чи задані units, clock domain, regression і rollover без звуження capture time? [Повнота, Spec §US2 scenario5; Data-model §Час та ідентифікатори]

## Часові та конкурентні межі

- [x] CHK013 Чи відокремлено threshold100мс, delivery10мс і необмежений цим budget час тексту? [Вимірюваність, Spec §FR-003; SC-002; D52]
- [x] CHK014 Чи явно не підміняє capture timeout майбутню політику cutoff/arming/watchdog05? [Узгодженість, Spec §FR-006; Q05]
- [x] CHK015 Чи задана реакція на пропущений deadline та епізод stuck, який завершився до обробки monitor? [Gap, Spec §US2; Plan §Stuck]
- [x] CHK016 Чи мають rearm і processing числові межі, початок відліку й обґрунтований запас? [Вимірюваність, Plan §Rearm/Processing; Spec §FR-005]
- [x] CHK017 Чи визначено початок/завершення active guard, зокрема final HIGH-overrange? [Покриття, Contract §Узгоджені межі epoch; Spec §FR-003]
- [x] CHK018 Чи узгоджені захист shared state, concurrent edge/arm та recovery ownership? [Повнота, Contract §Узгоджені межі epoch; Plan §Архітектура]
- [x] CHK019 Чи задані ownership/capacity буферів і незалежне від повної черги збереження fault? [Повнота, Data-model §Сутності; Spec §FR-003]
- [x] CHK020 Чи визначена достовірність line observation на час маскування ISR та після відновлення? [Gap, Plan §Архітектура крок6; Data-model §LineObservation]

## Діагностика та незалежність керування

- [x] CHK021 Чи однозначно задані частота/інтервал diagnostic attempts і відсутність blocking/history/burst? [Ясність, Spec §FR-004; Contract §Owner → diagnostics → USB]
- [x] CHK022 Чи відрізняються pending latest snapshot, committed record та неконтрольовані host buffers? [Узгодженість, Spec §Clarifications D52; Contract §Owner → diagnostics → USB]
- [x] CHK023 Чи однозначно визначені одиниці обліку overwrite/skip/write-drop, насичення й reset counters? [Gap, Spec §FR-004; Data-model §DiagnosticState]
- [x] CHK024 Чи достатньо визначені зміст diagnostics, framing/partial records та відсутність command source? [Повнота, Spec §FR-004; Contract §Owner → diagnostics → USB]

## Якість критеріїв прийняття та залежностей

- [x] CHK025 Чи задано, які timing boundaries обов’язково підтверджувати фізично в03, а які достатньо покрити host fixtures? [Ambiguity, Spec §FR-005/SC-003; Quickstart §Операторський стенд]
- [x] CHK026 Чи задане правило pass/fail з урахуванням невизначеності analyzer біля меж5/10мкс? [Gap, Spec §SC-003; Quickstart §Операторський стенд]
- [x] CHK027 Чи визначений max diagnostic load через фактичний підтримуваний payload, attempts і reader conditions? [Ambiguity, Spec §US3 scenario2; Quickstart §P2]
- [x] CHK028 Чи розділено вимоги до host/build/physical evidence і заборонено видавати непроведене за pass? [Повнота, Spec §FR-005/007; SC-004]
- [x] CHK029 Чи зафіксовані versions/profile/electrical gates без заяви про неперевірену сумісність? [Повнота, Plan §Technical Context; Spec §FR-007]
- [x] CHK030 Чи простежуються R05/R09/R11/R12 через FR/SC до видів acceptance evidence? [Простежуваність, Spec §Acceptance Traceability and Evidence; Plan §Verification and Traceability]
- [x] CHK031 Чи відокремлені чинні рішенняD51/D52, історичні вимоги та later-owned Q03/Q05/Q06? [Узгодженість, Spec §Scope/Clarifications; Research §RCH-10–13]

## Примітки

Результат кожного пункту й посилання на фактичний текст записуються нижче. Pre/post hooks відсутні: .specify/extensions.yml не існує. Tasks/Implement та апаратні дії не виконуються.


## Тріаж перед питаннями

A — 23 пункти вже задоволені; B — 6 очевидних виправлень застосовано; C — 2 рішення власника: CHK025 закрито A/D53, CHK026 закрито B/D54. Усі 31 пункт закрито. Посилання з номерами рядків — знімок прочитаного тексту до доповнень, назви розділів лишаються anchors.

### A — вже задоволено

- CHK001: already satisfied — spec.md:11–14, FR-006.
- CHK002: already satisfied — spec.md:12/16, FR-007.
- CHK003: already satisfied — spec.md:24, FR-001/002.
- CHK004: already satisfied — contracts/rc-capture.md:15–17.
- CHK006: already satisfied — spec.md:13, FR-002.
- CHK007: already satisfied — spec.md:30/50, US1.
- CHK009: already satisfied — spec.md:13, FR-002.
- CHK010: already satisfied — contracts/rc-capture.md:33–41.
- CHK011: already satisfied — spec.md:13, FR-006; contract:41.
- CHK012: already satisfied — data-model.md §Час та ідентифікатори.
- CHK013: already satisfied — spec.md:15, SC-002; contract:47/60.
- CHK014: already satisfied — spec.md:14/16, FR-006.
- CHK016: already satisfied — plan.md:39–40.
- CHK017: already satisfied — contracts/rc-capture.md:27/47.
- CHK018: already satisfied — contracts/rc-capture.md:21–25.
- CHK019: already satisfied — plan.md:35–36; data-model.md:15–17.
- CHK021: already satisfied — contracts/rc-capture.md:53–60.
- CHK022: already satisfied — spec.md:24; contracts/rc-capture.md:58.
- CHK024: already satisfied — contracts/rc-capture.md:59–62.
- CHK028: already satisfied — spec.md §FR-005/007, SC-004.
- CHK029: already satisfied — plan.md §Technical Context; spec.md FR-007.
- CHK030: already satisfied — spec.md §Acceptance Traceability and Evidence; plan.md:89–94.
- CHK031: already satisfied — spec.md:15/26–28; research.md RCH-10–13.

### B — застосовані виправлення

- CHK005: Уточнено expected HIGH stop vs unexpected truncation у spec US2, contract і model. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.
- CHK008: Уточнено незалежність invalid period від resource guard/error, без автоматичного fault лише через period. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.
- CHK015: Додано завершені stuck episodes та deadline_missed/lateness без backdated pass. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.
- CHK020: Додано observation continuity gap при masking/re-enable, без вигаданих edges/freshness. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.
- CHK023: Визначено різні одиниці overwrite/busy/write-drop та version-specific acknowledgment. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.
- CHK027: Визначено 384 B bench load із padding актуального record і фактичними bytes/attempts під backpressure. Evidence: spec/plan/data-model/contract/quickstart §Уточнення checklist; research §Checklist 2026-10-29.

### C — рішення власника

- CHK025: A, D53 — HJ physical acceptance + host exact boundaries/malformed; окремий генератор не блокує. Застосовано spec SC-003/Clarifications, plan, research, data-model, contract, quickstart та docs/decisions.md.
- CHK026: B, D54 — пряме порівняння E≤5/10 мкс, U≤1 мкс окремо без запасу. Застосовано spec FR-005/SC-003/Clarifications, plan, research, data-model, contract, quickstart і docs/decisions.md.

## Resolution (2026-10-29, before /speckit-tasks)

Guided review завершено: A — 23 already satisfied, B — 6 applied fixes, C — 2 рішення власника (CHK025 A/D53, CHK026 B/D54). Відкритих CHK немає; усі 31 позначено [x]. Попередніх speckit-analyze findings для03 немає. Physical evidence не отримано; цей checklist не hardware acceptance.

Commit/push не виконуються: AGENTS.md вимагає окремого завдання на ці Git-дії та прямо не дозволяє generic automatic actions skills скасовувати правила проєкту. Інші фази не запускаються.

### Рішення власника 1/2

D53 / CHK025-A (2026-10-29): для завершення 03 точні межі періоду 18/22 мс, точні граничні ширини та складні malformed-послідовності покриваються host fixtures. Фізично обов’язкові HJ Tester на 50 Гц у доступному діапазоні, width/interval accuracy, zero loss під diagnostic load, stuck HIGH/LOW та rearm/processing/fault budgets. Окремий генератор цих точних stimuli не блокує 03; host не підміняє зазначені обов’язкові фізичні докази. Q03/H02/H03 і receiver gate зберігаються.

### Рішення власника 2/2

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

### Підсумок Resolution

| Пункт | Вибір | Результат |
|---|---|---|
| CHK025 | A / D53 | HJ фізично на50 Гц; точні boundary/malformed — host; окремий генератор не блокує03. |
| CHK026 | B / D54 | E≤5/10 мкс; U≤1 мкс окремо, без додаткового запасу. |

Змінено в межах guided review: цей checklist, spec.md, plan.md, research.md, data-model.md, contracts/rc-capture.md, quickstart.md, docs/decisions.md та docs/open-questions.md. A evidence та шість B fixes наведені вище; незакритих analyze findings немає. Фізичні Q03/H02/H03/прилад і непроведені tests/build/measurements залишаються відкритими як execution/acceptance work, а не неузгоджені вимоги. Інші зміни користувача збережено.

Наступна окрема фаза: /speckit-tasks. Commit/push не виконано за AGENTS.md; жодних Tasks/Implement/flash/load дій не запускалося. Pre/post checklist hooks відсутні, .specify/extensions.yml не існує.


### Доповнення analyze-guided C1-A / D55

Вимірюваність SC-003 конкретизовано: усі pulses/intervals у коротких bounded RAM windows порівнюються offline після freeze/stop/export; окремі600с runs перевіряють втрати та часові бюджети. Production diagnostics latest-only не є per-pulse evidence. D54/B незмінне: directE≤5/10мкс, U≤1мкс окремо. Tasks T052–T054 додані зі стабільними IDs; фізичних результатів немає. Це доповнення наявних CHK025/026, не перевірка реалізації чи новий hardware pass.
