# Результат guided Analyze: 01-foundation

## Аналіз 2026-10-28, перед /speckit-implement

**Обсяг**: spec/plan/tasks, пов'язані research/data-model/contracts/quickstart,
checklist і чинні рішення. Виконано `speckit-analyze` всередині
`speckit-analyze-guided`; мінімальної severity floor та report-only не задано.

**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/01-foundation`.
Штатний `check-prerequisites.sh --json --require-tasks --include-tasks`
повернув `/Users/mac/Projects/Scopa/rc-pwm-bridge/specs/01-foundation` і наявний tasks.md.
Конституція v1.0.0 ухвалена, заповнена; її тіло не змінювалося.

### Початковий стан і порядок роботи

Початковий HEAD: `a8af92adf2715f91eaafb08c58c7b3860c6af0f1`, гілка `01-foundation`.
На початку tasks.md був незакоміченим файлом із SHA-256
`afb0c88277169416c47e78af257379cca3812daaee22d9025cc5f35616b8271a`.
Зібрано hashes 118 наявних файлів. До показу повного первинного звіту
перевірено незмінність усіх файлів: аналіз був read-only.
Редагування почалося після звіту, за дозволом самого виклику guided skill.

`.specify/extensions.yml` відсутній: перевірки before_analyze/after_analyze
не знайшли зареєстрованих hooks. Інші фази Spec Kit автоматично не запускалися.

### Метрики

| Показник | Початковий звіт | Після повторних проходів A–F |
|---|---:|---:|
| Вимоги | 24: 16 FR + 8 SC | 24: 16 FR + 8 SC |
| Завдання | 40 | 40 |
| Вимоги з хоча б одним завданням | 24/24, 100% | 24/24, 100% |
| Непов'язані завдання | 0 | 0 |
| Неоднозначності, що лишилися відкритими | 1 | 0 |
| Зайві дублікати вимог | 0 | 0 |
| Відкриті CRITICAL | 0 | 0 |
| Відкриті первинні знахідки | 4 | 0 |

Покриття 100% є покриттям ID вимог завданнями, не проходженням runtime-тестів
або SC. Первинна E1 була пропуском конкретного сценарію всередині покритих FR.
Усі 40 завдань лишилися `[ ]`; жодного не виконано цією фазою.

### Знахідки та їх розв'язання

Початкові посилання на рядки нижче стосуються стану до редагування.
Категорія 0 — не дефект; A — однозначне виправлення; B — рішення власника.

| ID | Severity | Категорія | Початковий доказ | Розв'язання та змінені файли |
|---|---|---|---|---|
| F1 | LOW | A | `spec.md:35–38`: «Tasks/Implement не запускалися»; завершення plan називало Tasks наступною фазою | Актуалізовано статуси й посилання на Tasks/Analyze, збережено історію фаз. `spec.md`, `plan.md`; рядки доказів у checklist синхронізовано. Закрито. |
| C1 | MEDIUM | B | `contracts/startup.md:24–26`: «очікувана похибка планувальника до одного циклу перевіряється окремо» без target-процедури в задачах | Власник обрав A; додано D23: номінальний poll 10 мс, мінімальна пауза 1 с, без гарантованої верхньої межі target lateness у 01. Окремий timing-test образ не додається. `docs/decisions.md`, spec/plan/research/data-model/startup/build-evidence/quickstart/tasks/checklist. Закрито. |
| E1 | MEDIUM | A | Startup contract вимагав перевірки невід'ємності clock перед cast; T032/T034 явно охоплювали regression/overflow, але не invalid/negative raw clock або clock-port failure | Уточнено checked conversion, time-validity/fault інтерфейс і terminal ClockFault без init/GPIO/setup чи вигаданого часу; додано явні fake-clock сценарії. `tasks.md` (T003/T004/T025/T026/T034), `contracts/startup.md`, `data-model.md`, `contracts/build-and-evidence.md`, `quickstart.md`, `plan.md`. Закрито. |
| C2 | LOW | A | `data-model.md:42` забороняв RTTI/exceptions; build contract і T005/T028 не прив'язували явні core flags до build-конфігурації, хоча T035 вимагав їх перевірки | Уточнено PRIVATE -fno-rtti/-fno-exceptions на host/target core, export compile_commands, окремі actual host/target audits та source review heap-залежностей. `tasks.md` (T005/T017/T019/T022/T028/T035/T036), `plan.md`, `contracts/build-and-evidence.md`. Закрито. |
| N1 | MEDIUM | A | Проміжна правка C2 включила actual target flag audit у host-задачу T035, що могло зробити host-перевірку залежною від E01 | Target audit залишено у T019/T022, host audit — у T036. Уточнено залежність T025 від shared checked-conversion T026 і приклад паралельності; DAG повторно перевірено. `tasks.md`, `contracts/build-and-evidence.md`. Закрито до завершення аналізу. |
| N2 | MEDIUM | A | Після уточнення E1 clock-fault snapshot не мав явного способу відрізнити невідомий час від нульового або останнього валідного timestamp | Додано now_valid: false для невдалого/невалідного sample, now_us — останній прийнятий час або 0 до першого валідного sample, без видавання за поточний вимір. Початкова відсутність sample сама не є ClockFault. `data-model.md`, `contracts/startup.md`, `contracts/build-and-evidence.md`, `tasks.md` (T003/T032/T034). Закрито. |

Первинне рознесення: **0 — 0, A — 3, B — 1**;
первинні severity: **MEDIUM — 2, LOW — 2, HIGH/CRITICAL — 0**.
Додаткові N1/N2: **A — 2, MEDIUM — 2**, обидві виправлені.
Загалом закрито шість записаних знахідок; відкладених або залишених нижче floor немає.

### Рішення власника

| Знахідка | Вибір | Наслідок |
|---|---|---|
| C1 | A, після пояснення poll та init retry, 2026-10-28 | D23: poll 10 мс — номінальний. Retry починається не раніше completion + 1 000 000 мкс і на першому доступному poll на/після deadline. Запізнілий tick видає одну спробу без catch-up; верхня межа target lateness у 01 не приймається як acceptance-критерій. |

П'ять спроб, ReadyDisabled/Exhausted, PWM LOW/permission=false, D20–D22 та
власник operational cutoff Q05 збережені. Числового RC timeout або нового
режиму роботи з навантаженням не додано. Історичні D01–D22 не переписано;
нове рішення D23 додано окремим рядком.

### Повторна перевірка A–F

- **A — дублювання:** FR-014/FR-015 і SC є різними зобов'язаннями щодо форми,
  непроведених перевірок і прийняття; зайвого дублювання для видалення не виявлено.
- **B — неоднозначність:** C1 узгоджено D23; незадані flash settings та фізичні
  числа H03 залишаються явними owned gates, а не прихованими defaults.
- **C — повнота:** clock validity і checked conversion мають сценарії; core flags
  прив'язано до build-файлів; допоміжні test paths зіставлено з планом.
- **D — конституція:** перевірено всі шість принципів у поточному обсязі.
  Немає незакритого конфлікту MUST. Попередній прямий дозвіл користувача D10
  щодо feature-гілок збережено; нових branch/апаратних дозволів Analyze не створює.
- **E — покриття:** 24/24 FR/SC мають завдання; unmapped tasks — 0.
  Для E1 додано саме пропущений clock-failure сценарій, не вигаданий pass.
- **F — узгодженість:** усі T001–T040 унікальні, формат і story labels коректні,
  граф ациклічний. Host T036 не залежить від target E01/T016, build T021 — від
  фізичних T012/T030/T031. T040 охоплює всі обов'язкові задачі через залежності.

Нових невиправлених знахідок після цих проходів немає. Форматні перевірки
посилань/таблиць і збереження захищених файлів виконуються як перевірки документів,
а не як host/target/physical evidence firmware.

### Змінені файли

- `docs/decisions.md` — лише нове D23.
- `specs/01-foundation/spec.md`, `plan.md`, `tasks.md`, `research.md`, `data-model.md`, `quickstart.md`.
- `specs/01-foundation/contracts/startup.md`, `contracts/build-and-evidence.md`.
- `specs/01-foundation/checklists/foundation-readiness.md` — актуальні рядки доказів та примітка синхронізації.
- `specs/01-foundation/analysis-resolution.md` — цей запис.

### Що залишається відкритим поза findings аналізу

E01, обґрунтований вибір flash mode/frequency/partition layout, H02/H03 і фактичні
host/build/config/physical acceptance results виконуватимуться у своїх задачах.
Відкладені RC/MD20A gates зберігають власників 03/04/06 за D21.
Жодних install/bootstrap, runtime-тестів, target build, flash чи вимірювань
цей workflow не виконував; ітерація 01 не оголошена завершеною.

Конституцію, AGENTS.md, PROJECT_FOUNDATION.md, input prompts, firmware/tests/scripts,
попередні raw ROM-докази та незалучені зміни користувача збережено.

### Наступний крок

`$speckit-implement` — лише за окремим явним запитом для `01-foundation`, з
`SPECIFY_FEATURE_DIRECTORY=specs/01-foundation`. Спочатку доступні підготовчі/
host/документальні задачі; залежний target-код — після E01. Операторські
T012/T030/T031 потребують окремих явних завдань і не дозволяються самим Implement.
Commit/push змінених артефактів — завершальний крок явно запитаного guided workflow;
публікується тільки поточна гілка `01-foundation`.
