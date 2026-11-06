# Результат speckit-analyze-guided: 04-pump-output

## Аналіз 2026-10-30, перед /speckit-implement

Обсяг: spec/plan/tasks і пов’язані model/contract/research/quickstart/checklist, конституція v1.0.0, поточні рішення та evidence boundaries01–03. Повний початковий звіт показано користувачу до будь-якого редагування; remediation дозволено явним викликом skill. Це документальний аналіз, firmware/host/target/hardware перевірки 04 не виконувалися.

### Початкові та повторно перевірені метрики

| Метрика | До | Після |
|---|---:|---:|
| Вимоги:13FR +6SC |19|19|
| Задачі |53|53|
| Вимоги з≥1 пов’язаною задачею |19/19|19/19|
| Формальне покриття |100%|100%|
| Непов’язані задачі |0|0|
| CRITICAL |0|0|
| Відкриті HIGH з початкового звіту |2|0|
| Відкриті MEDIUM з початкового звіту |2|0|
| Неоднозначності |2|0|
| Дублювання, що потребує виправлення |0|0|

Початкові категорії:0 —0, A —3, B —1. Під час синхронізації додатково виявлено й одразу усунено N1 [HIGH], категорія A. Початкові кількості HIGH2/MEDIUM2, разом 4; з N1 в історії remediation HIGH3/MEDIUM2, разом 5. Відкритих після повторного проходу немає. 100% — наявність coverage mapping, не hardware acceptance і не твердження про виконані задачі.

### Знахідки та виправлення

| ID | Severity | Категорія | Початковий фактичний текст / проблема | Рішення та перевірений результат | Файли |
|---|---|---|---|---|---|
| F1 | MEDIUM | A | plan.md:53: «Core calculated/effective0/permissionfalse» — неоднозначне скорочення; spec зберігає calculated duty02/03. | Явно зазначено, що calculated може бути ненульовим, effective=0/work_permitted=false; перевірено plan.md:53. Нового продуктового рішення немає. | plan.md |
| F2 | HIGH | A | contract:54: «Якщо LOW API неуспішний, LowUnconfirmed/fail» не розділяло помилку stop0 і невдачу повного fallback. | Будь-яка API error зберігає Fault/failed run. Новий LowUnconfirmed — лише за відсутності успішного повного LOW-шляху: stop0 або обох операцій set_level0→set_directionOUTPUT; старі reason bits залишаються sticky, початковий budget не перезапускається. Fallback success не є physical evidence; додано окремі fixtures. Перевірено contracts/pump-output.md:54, data-model.md:62, T028/T031. | plan.md, contracts/pump-output.md, data-model.md, quickstart.md, tasks.md |
| E1 | MEDIUM | A | T049 рахувала capture/діагностику, але явно не вимагала PWM у stress +1..99 с plateau із контракту:63. | T049 тепер включає кожен period за frequency/duty E/U, static endpoints/voltage/transitions у GPIO0 і PWM-вході MD20A під тим самим diagnostic load; додано FR-002/SC-002 та coverage mapping. Перевірено tasks.md:101. | tasks.md |
| U1 | HIGH | B | D61/model/quickstart задавали 600 MCU-секунд, тоді як успадковане фізичне прийняття вимагає 10 хв; узгодження clock domains не було. | Відповідь власника A, новий D62: RAM window601 MCU-секунда [t0+5 с,t0+606 с), output LOW на t0+605 с. Прийняття окремо вимагає T_obs−U_duration≥600000000000 нс між first/last matched falling, gapless source/узгоджені counts; самі MCU-секунди не дають pass. Перевірено spec.md:63, contracts/pump-output.md:113, T031/T035–T040/T049. | docs/decisions.md (лише D62), spec.md, plan.md, research.md, data-model.md, contracts/pump-output.md, quickstart.md, tasks.md, checklists/pump-output-boundaries.md |
| N1 | HIGH | A, нова | Після зміни U1 загальне правило export на Output Complete могло дозволити ранній export/freeze на 605 с до evidence end606 с. | Загальний контракт export і T034/T038 явно чекають окремого window freeze після 606 с; T031/T040 перевіряють заборону раннього export, T036 зберігає collection/max diagnostics після output LOW. Перевірено contracts/pump-output.md:76, T034/T036/T038. | contracts/pump-output.md, data-model.md, plan.md, tasks.md та D62 sync |

### Рішення власника

| Знахідка | Відповідь | Прийнято |
|---|---|---|
| U1 | A, 2026-10-30 | D62:601 MCU-секунда RAM + незалежний доказ≥600 фізичних секунд; output schedule D60 незмінний. Відхилено B — звузити 04 до номінальних600 MCU-секунд; C — відкласти. |

Історичні D01–D61 не переписано; новий D62 замінює лише первісні межі600 MCU-секунд у D61. Scope no-pump/one-session-per-boot за D59, RAM≤512B/stress≤3200B/accuracy≤9088B, transport384B/100 мс, GPIO3/0/без MCU DIR та межі M04 збережені. Фізичне lower-bound правило не змінює E/U критерії D54 або frequency/duty M04.

### Повторні detection passes A–F

| Прохід | Результат |
|---|---|
| A — Duplication |13 унікальних FR/6 унікальних SC. Повторні посилання на fault у spec/contract/tasks є різними рівнями простежуваності, не дубльованими продуктовими вимогами. Нових дублів не виявлено. |
| B — Ambiguity | F1/F2 уточнено; U1 розв’язано D62. Clock domain/end605 vs606/closure/fallback/presence semantics явні. Unresolved placeholders не знайдено. |
| C — Underspecification | D62 bounds/closure/export/lower-bound/method/provenance та negative fixtures задані. Потрібні шляхи є наявними або planned у Plan; додаткового GPIO/command/source немає. |
| D — Constitution | Scope/LOW/fault/core/SDK/evidence/workflow/openQ constraints дотримані. Жодного CRITICAL не виявлено. Фізична готовність залишається blocked, а не pass. |
| E — Coverage |19/19 мають задачі, жодної непов’язаної задачі. E1stress waveform та U1physicalcoverage мають явні implementation/test/operator tasks. |
| F — Inconsistency |53 послідовні ID,11 позначок[P],7 операторських задач. Залежності спрямовані назад без циклів, storylabels/path/checkbox format валідні. MCU601s та physical≥600s не змішуються; N1 закрито. |

Проведено read-only structural checks для IDs/labels/paths/DAG/coverage, локальних посилань, відсутності placeholders, меж RAM/canonical144B та 605/606 timing arithmetic; git diff whitespace check. Нових firmware тестів/builds/physical measurements не запускали. Hooks до/після Analyze: .specify/extensions.yml відсутній. Інших фаз не викликали.

### Покриття після виправлень

| Вимога | Є задача | Задачі (зокрема) |
|---|---|---|
| FR-001 | Так | T013, T016, T018, T020, T026, T043, T045 та ін. |
| FR-002 | Так | T016, T018, T023, T025, T047, T049, T052 |
| FR-003 | Так | T004, T008, T010, T014, T020, T032, T046 та ін. |
| FR-004 | Так | T004, T008, T009, T010, T011, T012, T014 та ін. |
| FR-005 | Так | T004, T005, T010, T014, T016, T017, T019 та ін. |
| FR-006 | Так | T004, T010, T029, T037, T050, T052 |
| FR-007 | Так | T004, T005, T010, T021, T028, T029, T031 та ін. |
| FR-008 | Так | T004, T030, T031, T033, T034, T038, T049 та ін. |
| FR-009 | Так | T003, T044, T046, T052 |
| FR-010 | Так | T001, T003, T044, T045, T046, T052, T053 |
| FR-011 | Так | T003, T023, T025, T039, T040, T044, T046 та ін. |
| FR-012 | Так | T002, T006, T009, T011, T012, T013, T015 та ін. |
| FR-013 | Так | T001, T007, T015, T017, T022, T024, T027 та ін. |
| SC-001 | Так | T008, T027, T041, T052 |
| SC-002 | Так | T025, T047, T049, T052 |
| SC-003 | Так | T025, T047, T052 |
| SC-004 | Так | T008, T015, T017, T024, T025, T031, T036 та ін. |
| SC-005 | Так | T031, T035, T036, T039, T040, T041, T049 та ін. |
| SC-006 | Так | T001, T006, T007, T015, T017, T024, T027 та ін. |

### Відкриті межі

Відкритих findings/deferred/below-floor немає. Q02/H02/H03, Q03 conditioning/capture physical evidence та Q06 MD20A revision/interface/jumper залишаються окремими фізичними gates. Q05 operational arming/watchdog→05; direction/power/current/thermal/emergencycutoff→06. 53 задачі залишаються unchecked; документальна готовність не приймає обладнання. Недостатній прилад, невідомий U чи неузгоджений anchor→blocked; виміряний невідповідний результат→fail.

### Змінені документи та завершення

spec.md, plan.md, research.md, data-model.md, contracts/pump-output.md, quickstart.md, tasks.md, checklists/pump-output-boundaries.md, цей analysis-resolution.md та docs/decisions.md (новий D62). Наявні prompts05–09/README та інша робота користувача збережені, конституція й попередні features не змінювалися. Commit/push цих пов’язаних документів у поточну гілку 04-pump-output виконується за explicit skill; сторонні зміни не включаються.

Наступний крок — окремий /speckit-implement для програмних задач 04; операторські задачі автоматично не виконуються. Відкритих CRITICAL немає.
