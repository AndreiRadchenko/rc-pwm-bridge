# Checklist готовності вимог до Tasks: 01-foundation

**Мета**: перевірити якість вимог і плану щодо меж 01, startup/fail-off,
контактного профілю після D20, відтворюваності та доказів.
**Створено**: 2026-10-28.
**Ітерація**: [spec.md](../spec.md); [plan.md](../plan.md).
**Глибина / аудиторія**: reviewer; власник продукту й рецензент перед `/speckit-tasks`.
**Примітка**: створено через `speckit-checklist` у межах
`speckit-checklist-guided`; це перевірка написаних вимог, не результат тестування firmware.
Наявний `requirements.md` є історичним результатом Specify й не замінюється.

## Повнота обсягу й актуальність

- [x] CHK001 Чи однозначно обмежено 01 одним локальним pump startup-каркасом без operational RC/PWM/arming? [Повнота] [Spec §Обсяг і межі рішень; FR-001]
- [x] CHK002 Чи відокремлено інваріанти v1 — 1000–2000 мкс, 1000 Гц, без калібрування — від функцій наступних етапів і відкладених платформ? [Ясність] [Spec §Обсяг і межі рішень]
- [x] CHK003 Чи актуальні статус фаз, джерела D18–D20 й формулювання про наявні артефакти після виконаного Plan? [Узгодженість] [Conflict] [Spec §Статус; §Межі фаз; SC-004]
- [x] CHK004 Чи однаково визначено RC=GPIO3, PWM=GPIO0 та постійну перемичку DIR–GND без GPIO1 і програмного DIR в усіх контрактах? [Узгодженість] [Spec §FR-005–FR-007; D20]

## Профіль, залежності та межі фізичного прийняття

- [x] CHK005 Чи узгоджено критерій погодження контактів US1.AC2 із достатністю pinout за D18 та відокремленням фізичного bias/живлення? [Узгодженість] [Conflict] [Spec §US1.AC2; FR-004–FR-005]
- [x] CHK006 Чи розрізнено власні profile/unit ID, невідому заводську ревізію, джерела даних і два статуси design/physical без заяв про всі SuperMini? [Повнота] [Spec §FR-004; SC-004]
- [x] CHK007 Чи однозначно визначено, які саме поля SC-004 обов'язкові для завершення 01, а які фактичні дані RC/MD20A приймаються у 03/04/06? [Межі прийняття] [Conflict] [Spec §SC-004; §Реєстр рішень Q03/Q06; Plan §H04/H05]
- [x] CHK008 Чи визначено склад числової методики H03, її погодження до вимірювань і чесний blocked-статус за відсутності приладу/меж? [Вимірюваність] [Spec §FR-013; SC-006; contracts/board-profile.md §GPIO-only прийняття 01]
- [x] CHK009 Чи відділено вибір зовнішніх 3,3 В від виміряної якості живлення та неперевіреного одночасного USB+external режиму? [Припущення] [Assumption] [Spec §FR-006; Plan §H02]
- [x] CHK010 Чи окремо описані software LOW, зовнішній bias до firmware та незалежне силове аварійне відключення? [Повнота] [Spec §FR-012; US3.AC4]
- [x] CHK011 Чи однозначно виключено трактування PWM LOW MD20A як coast, ізоляції або доведеного нуля витрати? [Ясність] [Spec §FR-012; quickstart.md §GPIO-only фізичні спостереження]

## Startup, відмови та діагностика

- [x] CHK012 Чи визначені LOW-first порядок, зупинка решти init після помилки output API та відмінність успіху API від фізичного LOW? [Сценарії відмов] [Spec §FR-007–FR-008; contracts/startup.md §Порядок однієї платформної спроби]
- [x] CHK013 Чи кількісно визначено retry count, відлік паузи, кінцеві стани й незмінність permission=false після успіху або вичерпання спроб? [Ясність] [Spec §FR-008; contracts/startup.md §Межі; §Переходи]
- [x] CHK014 Чи описано часову регресію, overflow, дубльований result і відновлення тільки новою сесією без прихованого auto reset? [Граничні випадки] [Spec §FR-008; contracts/startup.md §Переходи]
- [x] CHK015 Чи узгоджена вимога доступності локальної причини/стану з відмовою самого USB transport і відсутнім host, без послаблення fail-off? [Сценарії відмов] [Conflict] [Spec §US3.AC2; FR-008; data-model.md §Платформний результат і локальний snapshot]
- [x] CHK016 Чи однозначно визначена доступність поточного, зокрема термінального, стану після drop/від'єднання й повторного підключення console host? [Відновлення] [Ambiguity] [Spec §FR-008; contracts/startup.md §Діагностика; data-model.md §StartupSnapshot]
- [x] CHK017 Чи задані межі діагностики, допустима втрата історії та заборона console-команд керування без впливу на init/LOW? [Нефункціональні вимоги] [Spec §FR-008; contracts/startup.md §Діагностика]

## Відтворюваність і переносимі межі

- [x] CHK018 Чи зафіксовані SDK commit, tools, C++ baseline й конкретний host без заяв про неперевірені Nano або інші ОС? [Повнота] [Spec §FR-002; FR-010; research.md §RCH-01–RCH-02]
- [x] CHK019 Чи визначено власника й строк вибору flash mode/frequency/partition settings, без прихованого прийняття SDK defaults або заяви, що всі технічні вибори завершено? [Залежності] [Gap] [Spec §FR-002; FR-016; contracts/build-and-evidence.md §SDK/build-конфігурація]
- [x] CHK020 Чи однакова межа E01 перед залежною target-реалізацією/збіркою та явно дозволена підготовка самого bootstrap без циклічного блокування? [Узгодженість] [Conflict] [Spec §Реєстр рішень Q01; Plan §Технічний контекст; research.md §E01]
- [x] CHK021 Чи відрізнено майбутні реальні entry points і firmware root від уже наявних скриптів та заборонено побічне прошивання/операції з навантаженням? [Ясність] [Spec §FR-002–FR-003; quickstart.md §Передумови]
- [x] CHK022 Чи визначено змістовні host-сценарії, незалежність від SDK/MCU та окремий negative probe з ненульовим кодом без маскування результату suite? [Якість прийняття] [Spec §FR-010–FR-011; SC-003; contracts/build-and-evidence.md §Host acceptance]
- [x] CHK023 Чи достатньо точно описано докази вимкнених radios/network, щоб один рядок defaults не зараховувався як повне прийняття? [Вимірюваність] [Spec §FR-009; SC-005; contracts/build-and-evidence.md §SDK/build-конфігурація]

## Докази, відповідальність і workflow

- [x] CHK024 Чи чітко відокремлені ROM chip/flash/USB докази від непроведених build/host/startup/bias вимірювань та невиконаних SC? [Узгодженість доказів] [Spec §FR-014–FR-015; §Простежуваність прийняття й доказів]
- [x] CHK025 Чи охоплює простежуваність усі R01/R02/R08/R10/R12 та FR-001–FR-016 через сценарії, SC і місця майбутніх доказів, не вигадуючи T-IDs до Tasks? [Простежуваність] [Spec §SC-008; §Простежуваність прийняття й доказів; Plan §Перевірка й простежуваність]
- [x] CHK026 Чи кожен відкладений bootloader/upload/brownout/watchdog сценарій має конкретний owning iteration та межу перевірки замість невизначеного загального «04–06»? [Відповідальність] [Gap] [Spec §FR-013; SC-007; US3.AC4]
- [x] CHK027 Чи мають відкриті E01/H02–H05 і Q01–Q09 власників та точні залежні дії, не блокуючи незалежну роботу й не перетворюючи пропозиції на defaults? [Залежності] [Spec §FR-016; §Реєстр рішень; Plan §Рішення та відкриті execution gates]
- [x] CHK028 Чи задокументовані явний каталог ітерації, окремі фази, збереження змін, українська документація й обмеження Git/апаратних дій відповідно до поточного завдання? [Workflow] [Spec §Припущення й залежності; Plan §Перевірка конституції]

## Перебіг розгляду

Пункти перевіряються за фактичними рядками артефактів. `[x]` означає якість
вимоги підтверджено текстом (A), однозначно виправлено (B) або рішення власника
записано та застосовано (C). Це не статус виконання SC чи апаратного gate.
Початкове рознесення: A — 20; B — 6; C — 2. Рішення C розглядаються послідовно.

## Докази рознесення A/B

Рядки наведено після однозначних виправлень; A — already satisfied текстом, B — виправлено.

| Пункт | Категорія | Текстове підтвердження |
|---|---|---|
| CHK001 | A | `spec.md:32` — «- **Поза обсягом**: повний автомат керування, capture RC, робочий PWM, arming/recovery,» |
| CHK002 | A | `spec.md:29` — «- **Незмінні межі v1**: 1000–2000 мкс → 0–100%, PWM 1000 Гц, без автоматичного» |
| CHK003 | B | `spec.md:9` — «**Статус**: Specify/Clarify/Plan виконано; guided checklist перед Tasks узгоджено.» |
| CHK004 | A | `data-model.md:22` — «\| direction_mode \| `hardwired_low`: перемичка DIR–GND на MD20A за D20. `dir_pin`/програмного `dir_level` немає; GPIO1 не використовується, ж» |
| CHK005 | B | `spec.md:110` — «тоді контактний дизайн відповідає погодженому pinout і перевірці SoC-конфліктів» |
| CHK006 | A | `data-model.md:26` — «\| approval \| Окремі design/bench statuses, джерело/дата, область дозволеної роботи та fingerprint вмісту. \|» |
| CHK008 | A | `contracts/board-profile.md:67` — «максимальний допустимий PWM LOW, похибка/калібрування приладу, bandwidth,» |
| CHK009 | A | `contracts/board-profile.md:35` — «\| Живлення \| Дизайн: зовнішні 3,3 В на 3V3/GND. H02: фактичні напруги/живлення; одночасне USB+external power не вважається перевіреним. \|» |
| CHK010 | A | `spec.md:284` — «- **FR-012**: Апаратний опис ПОВИНЕН окремо визначати програмний LOW, зовнішній» |
| CHK011 | A | `contracts/board-profile.md:58` — «PWM LOW MD20A — brake. До схем H-bridge не додається зовнішній diode лише за» |
| CHK012 | A | `contracts/startup.md:50` — «3. Якщо assert LOW повернув помилку, зупинити решту кроків, зафіксувати failure» |
| CHK013 | A | `contracts/startup.md:16` — «\| Максимум спроб \| 5 загалом, включно з першою. \|» |
| CHK014 | A | `contracts/startup.md:39` — «\| Регресія часу / overflow deadline / порушення протоколу результатів \| `ClockFault` або явний terminal protocol fault, без init-спроб і доз» |
| CHK015 | B | `contracts/startup.md:95` — «Стан/причина зберігаються у snapshot незалежно від доступності виводу. Відсутність» |
| CHK017 | A | `contracts/startup.md:82` — «256 байтів. Він використовує `usb_serial_jtag_write_bytes` з timeout 0,» |
| CHK018 | A | `research.md:37` — «app/platform adapters — **C++17**. Host baseline цього етапу — macOS arm64 із» |
| CHK019 | B | `research.md:189` — «console isolation і схему профілю. Flash mode/frequency/partition layout ще не» |
| CHK020 | B | `plan.md:39` — «\| Python closure \| E01: full exact-version/hash lock і wheelhouse створюються/відтворюються до залежної target-реалізації/збірки; зараз їх н» |
| CHK021 | A | `quickstart.md:8` — «**Команди нижче зараз не виконувалися й ще не готові до запуску.** Тут не наведено» |
| CHK022 | A | `contracts/build-and-evidence.md:70` — «CTest запускає справжній suite. Negative probe — окремий executable із навмисно» |
| CHK023 | A | `contracts/build-and-evidence.md:55` — «Перевірка відсутності radios: component/dependency list і linked archives не містять» |
| CHK024 | A | `spec.md:387` — «дала часткові докази ідентичності для SC-004; сам SC-004 і решта SC ще не прийняті.» |
| CHK025 | A | `spec.md:383` — «\| R02,R08,R12 / FR-016 \| US1.AC1–US1.AC3, US2.AC2 \| SC-001, SC-004, SC-008 \| Огляд рішень і меж блокування; `verification/board-review.md`, » |
| CHK026 | B | `spec.md:358` — «### Власники відкладених startup-сценаріїв (FR-013 / SC-007)» |
| CHK027 | A | `plan.md:62` — «\| H04 / Q03, 01/03 \| RC levels/conditioning до підключення; source timing/loss до capture acceptance 03 \| Blocked для RC-підключення; RC не » |
| CHK028 | A | `spec.md:406` — «`SPECIFY_FEATURE_DIRECTORY=specs/01-foundation`. За D10 під час Specify дозволені» |

CHK007 — C, рішення A, 2026-10-28, D21: SC-004 у 01 охоплює SuperMini;
RC/MD20A — документальні вимоги та pending/gates 03/04/06. H02/H03 і SC-006 не
скасовано. Застосовано до spec/plan/research/data-model/contracts/quickstart та
поточних реєстрів рішень/питань.

CHK016 — C, рішення B, 2026-10-28, D22: лише нові snapshot-події одноразово;
consumer poll 100 мс не є heartbeat. Drop/reconnect не спричиняє replay, тому
поточний/термінальний стан може не бути видимим до нової події або окремо
дозволеного reset. Застосовано до spec/plan/research/data-model/startup/build-evidence/
quickstart і додано D22; невиконаних пунктів checklist більше немає.

## Resolution (2026-10-28, before /speckit-tasks)

**Результат перевірки якості вимог: 28/28 `[x]`.** A — 20; B — 6; C — 2.
Це не прийняття SC, не закриття E01/H02–H05 і не завершення ітерації 01.

| Пункт C | Вибір власника | Записане рішення |
|---|---|---|
| CHK007 | A | D21: фізичний SC-004 у 01 охоплює SuperMini; RC/MD20A — документальні вимоги й gates 03/04/06. H02/H03 та SC-006 залишаються обов'язковими. |
| CHK016 | B | D22: одноразові нові події без heartbeat/replay; після drop/reconnect останній стан може не з'явитися до нової події чи окремо дозволеного reset. |

**Однозначні виправлення B:**

- CHK003 — актуалізовано статуси Specify/Clarify/Plan, поточну фазу та посилання на D20.
- CHK005 — погодження контактів за pinout відокремлено від фізичного bias/живлення, як уже визначено D18.
- CHK015 — snapshot відокремлено від доступності USB-виводу; відсутність host не є init failure, transport setup failure — є.
- CHK019 — flash mode/frequency/partition layout явно лишено невибраними з власниками Q01/Q02 і межею до target configure; мовчазні defaults заборонено.
- CHK020 — вирівняно E01 до залежної target-реалізації/збірки та дозволено підготовку самого bootstrap для закриття gate.
- CHK026 — конкретизовано власників bootloader/upload (04, силовий захист 06), brownout (06), watchdog recovery (05/06).

**Підтверджені текстом A:** CHK001, CHK002, CHK004, CHK006, CHK008–CHK014,
CHK017, CHK018, CHK021–CHK025, CHK027, CHK028. Текстові докази наведено вище.
Попереднього звіту Analyze немає; закриття його findings не заявляється.

**Змінені артефакти:** цей checklist; `spec.md`, `plan.md`, `research.md`,
`data-model.md`, `quickstart.md`, `contracts/startup.md`, `contracts/board-profile.md`,
`contracts/build-and-evidence.md`; поточні `docs/decisions.md` (нові D21/D22)
та `docs/open-questions.md`. Історичні рішення не переписувалися.

**Залишається відкритим для реалізації/прийняття:** E01, вибір flash build settings,
H02/H03 SuperMini та майбутні інтерфейсні gates 03/04/06. Host-тести, target build,
config review, flash і фізичні вимірювання в цьому workflow не виконувалися.
Raw ROM-докази, конституцію, початковий checklist, input prompts і firmware збережено.

**Hooks:** `.specify/extensions.yml` відсутній до та після генерації; hooks не запускалися.
**Наступна фаза:** `/speckit-tasks` тільки за окремим запитом із
`SPECIFY_FEATURE_DIRECTORY=specs/01-foundation`; `tasks.md` не створювався.
