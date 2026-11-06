# Розв’язання cross-artifact analysis05

## Аналіз 2026-11-06, перед /speckit-implement

**Обсяг**: явно викликаний `speckit-analyze-guided`; початковий `speckit-analyze` залишався read-only до повного звіту. Українські вимоги/дизайн/tasks, без реалізації, build або hardware. `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming`; prerequisites повернули абсолютний FEATURE_DIR05 та research/data-model/contracts/quickstart/tasks. Гілка `05-failsafe-arming` не змінювалась.

### Початкові та повторні метрики

| Метрика | До виправлень | Після повторної перевірки |
|---|---:|---:|
| FR |17|17|
| SC із виконуваною роботою/evidence |8|8|
| Вимоги разом |25|25|
| Задачі |70|70|
| Вимоги з≥1 задачею |25/25 (100%)|25/25 (100%)|
| Непов’язані задачі |0|0|
| Відкриті findings |3|0|
| CRITICAL |0|0|
| HIGH |1|0|
| MEDIUM |2|0|
| LOW |0|0|
| Неоднозначності |1|0|
| Суттєві дублювання |0|0|
| Parallel [P] tasks |17|17|

Формальне покриття — простежуваність, не actual acceptance. Усі70 tasks лишаються unchecked; host/build/physical05 не виконані.

### Знахідки, triage та розв’язання

| ID | Рівень | Група | Фактичний початковий текст / дефект | Розв’язання | Змінені файли |
|---|---|---|---|---|---|
| I1 | HIGH | A | tasks.md:84/93 вимагали US2 hold/recovery і checkpoint T024; tasks.md:111/T028 лише в US3 задавав «Мігрувати current input/history та operational observer path». Research RCH-03 підтверджував наявний sticky-OR/terminal inhibition. | Спільну Operational05 міграцію перенесено в T013 до US2; T028 інтегрує Fly зі вже мігрованим шляхом. Transitive dependency T013→T017→US2/T024 названа явно, нумерація не змінена. Legacy semantics збережені. | tasks.md, plan.md |
| U1 | MEDIUM | B | data-model.md:61 містив «Timing journal≤64 records×48B+header128B», але не event selection/start/freeze; T022/T050 не закривали цей lifecycle. | Власник обрав рекомендований event journal сценарію; нове D76. RAM accepted anchor без per-pulse history; transitions/API brackets/all failures/late/retry/D75 closure; повторні успіхи без зміни стану не записуються. Compile-time start/final checked freeze,≤64 records із anchors/API, missing provenance/result/overflow→incomplete, без впливу на control. Schema деталізується T022 до collector/fixtures, T050 використовує ту саму. | docs/decisions.md (новий D76), spec.md, plan.md, research.md, data-model.md, contracts/operational-control.md, quickstart.md, tasks.md, checklists/operational-boundaries.md |
| C1 | MEDIUM | A | spec US2.AC1 вимагав hold при age<T; quickstart P3/tasks T063 мали «L+U≤1с» без явного відхилення передчасного physical LOW. | Healthy P3 має окремі verdicts hold-before-T та maximum cutoff; достовірний prematureLOW→fail, unknown age/waveform correlation/uncertainty→blocked hold-part, combined P3 не pass за самою верхньою межею. Exact T±1 host і числові budgets незмінні; не вводиться безпідставний physical500мс threshold. T023/T047/T051/T058/T063 уточнено. | spec.md, plan.md, contracts/operational-control.md, quickstart.md, tasks.md, checklists/operational-boundaries.md |

Підсумок triage:0=0, A=2, B=1. Усі A виправлено до запитання B. Порога severity/report-only/відкладень користувач не задавав.

### Рішення власника U1

Обрано **«Журнал подій сценарію (Recommended)»**. Явна відповідь: «записувати переходи, пов’язані API brackets, failures і retry/D75 closure; повторні успіхи без зміни стану не записувати. Accepted anchor оновлюється в RAM; start/freeze визначає compile-time fixture».

Записано D76 і C05-05. Valid0% D75 closure є обов’язковою подією навіть за незмінного LOW. Посилання на accepted anchor веде на immutable запис, а не перезаписувану RAM-копію. Freeze чекає належних подій/pending fault-barrier перевірки; пізня належна пропущена подія не стає прихованим pass. Відкладених рішень немає. Це не новий operational mode, command source або production recorder; D64–D75 не переобрано.

### Повторні detection passes A–F

| Pass | Повторна перевірка й результат |
|---|---|
| A — duplication | Суттєвих дублів немає. Spec визначає policy, contract/data-model — lifecycle, tasks — work/evidence; повторні references узгоджені. |
| B — ambiguity | U1 має явний D76; placeholder markers відсутні. Невідомі physical inputs/метод/U/reset anchor залишені scoped gates, не guessed defaults. Concrete wire layout належить T022 до affected implementation. |
| C — underspecification | Event selection, immutable accepted anchor, start/final checked freeze, overflow і reset evidence задані. Нові test/SDK helper шляхи лишаються всередині обраних Plan embedded boundaries. |
| D — constitution | Поточнаv2.0.0/D74; усі шість gates враховані. One channel/native IDF/portable core, Test default/Fly explicit, no arming/zero handshake/manual input-loss clear/persisted duty, technical/watchdog/input loss окремі. Governance body, історичні ADR і specs01–04 не змінювались. CRITICAL0. |
| E — coverage |17 FR+8 SC мають tasks, усі70 tasks пов’язані з FR/SC; C1 має host/parser/operator checks, U1 — T022/T046/T047/T050/T051, I1 — T013 перед T024. Operator evidence не зараховано за host/build. |
| F — consistency | Точні T/1с/19–20мс/watchdog/retry/RAM limits не змінені; D75 closure не фільтрується, D76 lifecycle узгоджено в усіх owning artifacts. Task IDs/story labels/dependencies і17[P] у6 незалежних file groups збережені. |

I1/U1/C1 закрито; нових відкритих findings після повторного проходу немає. Під час редагування додатково уточнено immutable anchor reference, active-scenario scope, pending-fault-before-freeze та schema-before-fixtures — вони вже включені в остаточні правила, не лишені прихованими прогалинами.

Документальні checks:70 послідовних unchecked T-ID, правильні US labels/шляхи,17[P] у6 групах без спільних редагованих файлів;70 локальних Markdown-посилань feature мають наявні targets. `git diff --check` без помилок. Hash-порівняння зі станом до guided review обмежило зміни10 пов'язаними документами: новий resolution/tasks, owning artifacts/checklist і decisions з одним новим D76. Конституція, installed skills/templates, input prompts, specs01–04, firmware/tests та решта user work не змінені; старі ADR entries з Git HEAD збережено дослівно. Жодний firmware test/build/physical run цим не заявляється.

### Докази застосованих правок

- I1: [tasks.md:68](tasks.md); [tasks.md:111](tasks.md); [tasks.md:228](tasks.md).
- U1: [data-model.md:63](data-model.md); [tasks.md:91](tasks.md); [tasks.md:154](tasks.md).
- C1: [spec.md:193](spec.md); [quickstart.md:66](quickstart.md); [tasks.md:186](tasks.md).

### Межі та завершення

- Q02/H02/H03, Q03, Q06, actual source/receiver gates, придатний інструмент/U/незалежний reset anchor і фізичне прийняття05 залишаються pending/blocked у своїх межах. HJ-only не приймає receiver/radio-loss і не блокується відкладеним receiver gate.
- G05 закрито D74 раніше. D76 і цей огляд не є реалізацією, waveform, host/build pass або готовністю насоса06.
- Before/after analyze hooks: `.specify/extensions.yml` відсутній. Інші Spec Kit фази й device/flash/reset/load не запускалися.
- Commit/push у поточну гілку входять у явно викликаний `speckit-analyze-guided`, step5. Пакет містить пов’язаний раніше створений tasks.md, виправлення owning artifacts, новий D76 та цей resolution; unrelated files не включаються.
- Наступна окремо запитувана фаза — `/speckit-implement`; потрібні відповідні software/environment gates, а operator дії мають окреме явне доручення й physical readiness.
