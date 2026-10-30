# Checklist якості вимог: межі виходу насоса04

**Мета**: reviewer-перевірка повноти, ясності, узгодженості й вимірюваності вимог до04 перед Tasks; це не результати реалізації або фізичного прийняття.\
**Створено**: 2026-10-30\
**Feature**: [spec.md](../spec.md)\
**Аудиторія**: власник і рев’юер перед `/speckit-tasks`.\
**Глибина**: reviewer.\
**Фокус**: стендовий дозвіл/fault; single owner/handoff; вимірюваність; bounded evidence/diagnostics.

Створено через speckit-checklist у явно викликаному speckit-checklist-guided. Вхід базового skill: `pump-output-boundaries — reviewer depth, audience: власник/рев’юер перед speckit-tasks; focus: стендові дозволи/fault; single-owner/handoff; вимірюваність API/physical acceptance; evidence/diagnostic protocols; must-have: constitutionI–VI, D59/D60, preserve02/03, no pump/no GPIO/no guessed physical pass. Write Ukrainian.`

## Повнота та межі

- [x] CHK001 Чи явно обмежено04 одним RC-входом/виходом насоса на погодженому профілі, із DIR–GND та без насоса й розширень07–09? [Повнота, Spec §Обсяг і межі рішень; FR-001]
- [x] CHK002 Чи розрізнено normalLOW, стендовий дозвіл, calculated/effective duty ядра та запит фізичного виходу? [Ясність, Spec §FR-003,FR-008; Модель §DiagnosticSnapshot04]
- [x] CHK003 Чи узгоджено правило нового boot у D59 в усіх чинних сценаріях, а D58 позначено як замінену історію? [Узгодженість, Spec §Уточнення,FR-004; Plan §Короткий опис]
- [x] CHK004 Чи описано різницю між reader reconnect і справжнім app reset без появи RC/USB командного джерела? [Покриття, Spec §FR-004; Контракт §Скінченний сценарій і build identity]
- [x] CHK005 Чи однозначний стан normal при fault: Disabled для справного normal і видимий terminal fault при помилці без приховування причини? [Conflict, Spec §FR-003,FR-005; Модель §OutputSession і переходи]
- [x] CHK006 Чи явно успадковано validation-before-clamp, invalid hold, last-valid і valid-under-latch без нового operational timeout? [Узгодженість, Spec §FR-006; Plan §Перевірка конституції]
- [x] CHK007 Чи задані terminal/no-repeat, stale generation та неможливість recovery через resync/reader у тому самому boot? [Повнота, Spec §US1.AC3–4; Контракт §Вхід policy та пріоритети]
- [x] CHK008 Чи чисельно визначені initialLOW/readiness deadline і наслідок попередньої невдалої init-спроби? [Ясність, Контракт §Вхід policy та пріоритети; Модель §BootContext і ownership]

## Володіння та відмови

- [x] CHK009 Чи визначено одного власника GPIO0/LEDC, одноразовий handoff та заборону конкурентного startup writer? [Повнота, Plan §Архітектура та порядок роботи; Контракт §Апаратний adapter]
- [x] CHK010 Чи охоплює правило серіалізації fault усі операції, здатні видати ненульовий рівень, включно зі сталим HIGH endpoint? [Gap, Spec §FR-005,FR-007; Контракт §Вхід policy та пріоритети]
- [x] CHK011 Чи визначено результати часткового setup/застосування/cleanup та LowUnconfirmed, без видавання API success за фізичний cutoff? [Повнота, Spec §FR-005; Контракт §Апаратний adapter]
- [x] CHK012 Чи визначені equality/late boundaries, no-catch-up та помилки часу/переповнення generation до обчислень? [Ясність, Контракт §Скінченний сценарій і build identity; Модель §OutputSession і переходи]
- [x] CHK013 Чи визначено, що переповнений evidence buffer не блокує LOW і не спричиняє рекурсивне журналювання fault? [Gap, Spec §FR-005,FR-012; Контракт §Bounded вимірювальні записи та зіставлення часу]
- [x] CHK014 Чи описано failure path, коли LOW перед accuracy freeze/stop capture не підтверджений? [Gap, Spec §FR-005,SC-004; Контракт §Bounded вимірювальні записи та зіставлення часу]

## Вимірюваність критеріїв

- [x] CHK015 Чи розділені MCU-clock API1/2мс і прямий фізичний112мс критерій без недоведеного cross-domain pass? [Вимірюваність, Контракт §Часові межі M04; Plan §Вимірювання та compatibility]
- [x] CHK016 Чи однозначно означає LOW у112мс критерії сталий terminalLOW, а не звичайну LOW-частину проміжного PWM? [Ambiguity, Spec §SC-004; Контракт §Часові межі M04; Quickstart §P4]
- [x] CHK017 Чи задані окремі E/U та правило pass для frequency/duty/voltage, без усереднення чи підгонки допусків? [Вимірюваність, Spec §FR-011; Контракт §Фізичні допуски M04]
- [x] CHK018 Чи визначені stable/transition windows та clock-tolerance формула для довгих plateau? [Ясність, Контракт §Фізичні допуски M04; Plan §Параметри M04 та evidence builds]
- [x] CHK019 Чи визначено обсяг startup/reset/bootloader/upload спостережень і межі методу до firmware? [Покриття, Spec §FR-009,SC-003; Контракт §Фізичні допуски M04]
- [x] CHK020 Чи містить фізичний trace початкові рівні, повні часові bounds і ознаки безперервності, достатні для static endpoints без edges? [Gap, Spec §SC-003; Контракт §Offline physical input; Quickstart §P5]
- [x] CHK021 Чи відокремлено128-candidate accuracy/freeze від повної waveform послідовності та600с loss evidence? [Узгодженість, Spec §SC-004,SC-005; Plan §Вимірювання та compatibility]

## Докази та діагностика

- [x] CHK022 Чи задані bounded/latest-only/no-command semantics, payload/spacing/drop counters та committed-record reconnect caveat? [Повнота, Spec §FR-008; Контракт §Діагностичний запис rc04]
- [x] CHK023 Чи визначено незалежне від reader збереження start/end counter evidence600с вікна при slow/absent USB? [Gap, Spec §SC-005; Контракт §Offline physical input; Quickstart §P5]
- [x] CHK024 Чи визначено правило закриття600с вікна, віднесення пізно оброблених samples та зіставлення меж із джерелом без припущення30000? [Ambiguity, Spec §SC-005; Контракт §Offline physical input; Модель §OutputEvidence тільки bench]
- [x] CHK025 Чи задані стабільні wire-коди станів/операцій і обов’язковість trace-step, достатні для однозначного independent parser? [Gap, Модель §OutputSession і переходи,OutputEvidence тільки bench; Контракт §Діагностичний запис rc04]
- [x] CHK026 Чи документовані canonical widths/endian/digest та native/serialized/RAM межі як різні поняття? [Повнота, Модель §OutputEvidence тільки bench; Контракт §Bounded вимірювальні записи та зіставлення часу]
- [x] CHK027 Чи узгоджено збереження raw status кожної операції в record, а first/fallback summary — у header? [Conflict, Модель §OutputRequest та результат adapter,OutputEvidence тільки bench; Контракт §Апаратний adapter]
- [x] CHK028 Чи задані boot/build identity, handling partial/mixed archives та provenance без заяви глобальної унікальності timestamps? [Повнота, Модель §OutputEvidence тільки bench; Quickstart §P5]

## Залежності та готовність

- [x] CHK029 Чи кожний Q02/Q03/Q06 gate має власника й межу, а невідомий receiver не блокує HJ-only acceptance? [Assumption, Spec §Реєстр відкритих меж; Plan §Перевірка конституції]
- [x] CHK030 Чи збережено pinned environment/переносимість, заборону нових GPIO/радіо/NVS і межі недоведених hardware claims? [Узгодженість, Spec §FR-012; Plan §Технічний контекст]
- [x] CHK031 Чи всі FR мають acceptance/evidence traceability з окремими host/build/physical статусами та meaningful verification вимогою? [Повнота, Spec §FR-013,SC-006; Plan §Перевірки та простежуваність]
- [x] CHK032 Чи явно відкладено Q05/watchdog/power/load/напрямок і07–09 без їх мовчазного прийняття або блокування unrelated04? [Узгодженість, Spec §Обсяг і межі рішень; Quickstart §Окремі результати та завершення]
- [x] CHK033 Чи узгоджено закритий технічний M04 у реєстрі та evidence-таблиці spec, зі збереженими відкритими фізичними перевірками? [Conflict, Spec §Реєстр відкритих меж,Простежуваність прийняття та докази]

## Розбір пунктів перед рішенням власника

A — вже задоволено текстом; B — внесене однозначне виправлення; C — потрібне рішення. Позначки [x] означають якість вимог, а не виконаний firmware/hardware тест.

| Пункт | Категорія | Доказ у фактичному тексті / результат |
|---|---|---|
| CHK001 | A | `spec.md:18` — «- **Погоджений профіль**: `c3-supermini-pinout-01`, unit `c3-supermini-188b0e93032c`; RC=GPIO3, PWM=GPIO0, зов…». Вже задоволено. |
| CHK002 | A | `spec.md:21` — «- **Розрізнення дозволів**: у звичайній збірці 04 робочий дозвіл ядра завжди false, ефективна команда 0%, фізи…». Вже задоволено. |
| CHK003 | A | `spec.md:177` — «D59 дозволяє одну нову сесію після кожного app boot, включно з reset, і замінює power-cycle-only D58. Причина…». Вже задоволено. |
| CHK004 | A | `spec.md:129` — «- **FR-004 (R01,R08,R11)**: Окремо обрана стендова збірка після кожного app boot, включно з reset будь-якої пр…». Вже задоволено. |
| CHK005 | B | `data-model.md:29` — «¦ Disabled ¦ LOW,false ¦ Справний Normal; fault→Fault/LOW, без bench permission ¦…». Узгоджено Fault для normal зі спільним fault-priority; Disabled лишається справним normal. |
| CHK006 | A | `spec.md:131` — «- **FR-006 (R04,R05,R08)**: Система має зберігати validation-before-clamp, карту/rounding без навчання/фільтра…». Вже задоволено. |
| CHK007 | A | `contracts/pump-output.md:15` — «`request_generation` зростає при зміні запиту; fault має пріоритет над generation. Є один serialized writer po…». Вже задоволено. |
| CHK008 | A | `contracts/pump-output.md:13` — «5. Перший owner poll фіксує t0. InitialLow триває5 000 000мкс від t0. Якщо readiness не досягнуто до t0+5с або…». Вже задоволено. |
| CHK009 | A | `plan.md:48` — «3. На ReadyDisabled одноразовий handoff передає output/control ownership існуючому capture owner. До цього GPI…». Вже задоволено. |
| CHK010 | B | `contracts/pump-output.md:15` — «Перед кожною операцією, здатною встановити ненульовий вихід, включно з set/update та stop idle1 для100%, owner…». Правило sticky ordering/recheck поширено на stop idle1 для staticHIGH. |
| CHK011 | A | `contracts/pump-output.md:42` — «- На будь-якій API error спершу policy terminal fault/permissionfalse та інвалідація generation. Спробувати st…». Вже задоволено. |
| CHK012 | A | `contracts/pump-output.md:26` — «Deadlines абсолютні від t0, не накопичують drift від часу poll. Плановий перехід виконується лише на першому p…». Вже задоволено. |
| CHK013 | B | `contracts/pump-output.md:71` — «Повний buffer→EvidenceOverflow/Fault/LOW, неповний/overflow архів не дає timing pass. Переповнення логується s…». Overflow header/append-stop не блокують LOW/fallback; recursion заборонена, неповний архів failed. |
| CHK014 | B | `contracts/pump-output.md:78` — «Якщо LOW не встановлено навіть fallback, LowUnconfirmed має пріоритет: нормальний evidence-freeze/stop не вико…». LOW error/LowUnconfirmed не стає успішним freeze; explicit stop залежить від checkedLOW. |
| CHK015 | A | `contracts/pump-output.md:54` — «¦ Явний fault у owner→LOW API ¦ Checked LOW API success не пізніше1мс від першого застосування fault у seriali…». Вже задоволено. |
| CHK016 | B | `contracts/pump-output.md:92` — «T_cut — max(T0, початок останньої безперервної LOW-ділянкиGPIO0), яка зберігається протягом повної terminal об…». 112мс означає сталий terminalLOW із10с guard, не штатну LOW-фазу PWM; задані T0/T_cut/preconditions. |
| CHK017 | A | `contracts/pump-output.md:62` — «1. Для кожного стабільного періоду проміжних duty: ¦f−1000Гц¦≤1Гц та ¦d−номінал¦≤0,1 відсоткового пункту. U_f≤…». Вже задоволено. |
| CHK018 | A | `contracts/pump-output.md:63` — «2. Основні stable windows: у waveform від+1с до+9с кожного10с plateau; у stress від+1с до+99с кожного100с plat…». Вже задоволено. |
| CHK019 | A | `contracts/pump-output.md:66` — «5. Startup waveform до першої навмисної nonzero команди: прилад захоплює подію з pretrigger≥10мс, повну boot/i…». Вже задоволено. |
| CHK020 | B | `contracts/pump-output.md:94` — «CSV містить тільки edges; metadata version1 обов’язково має acquisition_start_ns, acquisition_end_ns, initial_…». Для edge-onlyCSV задані initial levels/acquisition bounds/observable windows/gaps; emptyCSV сам не доказ. |
| CHK021 | A | `plan.md:65` — «¦ Accuracy ¦ Waveform sequence; збір128 candidates03 починається на+1с вибраного trace step. На freeze LOW пер…». Вже задоволено. |
| CHK022 | A | `contracts/pump-output.md:82` — «Нормальна latest-only semantics03 збережена:≤10 записів/с,spacing≥100000мкс,transport≤384B,TX512/RX128,one com…». Вже задоволено. |
| CHK023 | C | `quickstart.md:78` — «window[t0+5с,t0+605с) має рівно600с. Для active/slow/absent reader — окремі runs; record attempts≤10/с з min10…». Закрито C1-A / D61: bounded RAM window evidence, capture продовжується; normative details у data-model та контракті. |
| CHK024 | C | `contracts/pump-output.md:88` — «Physical600с source/counter window має явне погоджене зіставлення pulse boundaries/boot і actual counter check…». Закрито C1-A / D61: bounded RAM window evidence, capture продовжується; normative details у data-model та контракті. |
| CHK025 | B | `data-model.md:81` — «phase:0 Disabled,1 InitialLow,2 Step0,3 Step1,4 Step2,5 Step3,6 Step4,7 Step5,8 Complete,9 Fault,a EvidenceFro…». Задані stable wire codes та explicit required accuracy trace-step без default. |
| CHK026 | A | `data-model.md:71` — «canonical big-endian widths4,1,1,1,1,4,8,8,8; canonical record36B, native alignment≤40B). FNV offset basis0xcb…». Вже задоволено. |
| CHK027 | B | `data-model.md:44` — «ApplyResult: generation, requested mode, observed API-result mode (Unknown/Low/Pwm/High), operation kind (Init…». Raw sdk_status кожної операції в record; first/fallback у header тільки summary з presence semantics. |
| CHK028 | A | `data-model.md:73` — «Boot label/start time не гарантує глобальної унікальності: оператор зберігає кожен boot в окремий raw-файл із…». Вже задоволено. |
| CHK029 | A | `spec.md:41` — «¦ Q02/H02/H03 — 01, залежне прийняття 04 ¦ Джерело 3,3 В, PWM bias, startup waveform, режим USB/external power…». Вже задоволено. |
| CHK030 | A | `plan.md:19` — «¦ SDK/tools ¦ Native ESP-IDF v5.5.6, commit880a203ae603d08a94d06aa01e0d6761958e5e9e; GCC14.2.0 esp-14.2.0_2026…». Вже задоволено. |
| CHK031 | A | `spec.md:138` — «- **FR-013 (R12)**: Кожний FR має бути пов'язаний зі сценарієм/SC та доказом або явно відкритою перевіркою. До…». Вже задоволено. |
| CHK032 | A | `spec.md:17` — «- **Поза обсягом**: робоче RC→насос керування, operational arming, freshness timeout, повне recovery й watchdo…». Вже задоволено. |
| CHK033 | B | `spec.md:166` — «not_run; M04 визначено D60, фізичні gates відкриті ¦…». У spec evidence-таблиці M04 позначений обранимD60; physical gates залишені відкритими. |

**Проміжний результат**: A22, B9, C2;31/33 закрито. C2 об’єднано в одне рішення щодо600с доказів за недоступного USB-читача. Фізичні Q02/Q03/Q06 не закриті цією перевіркою.


## Resolution (2026-10-30, before /speckit-tasks)

**Завершено:33/33.** Початкова класифікація: A22 (вже задоволено), B9 (очевидні виправлення), C2 (одне об’єднане рішення власника). Це закриття якості вимог; firmware/build/physical pass не заявляються.

| Пункти | Рішення власника | Результат |
|---|---|---|
| CHK023,CHK024 | C1-A, відповідь «A», D61 | Незалежний від USB bounded RAM summary600с, capture після вікна продовжується. Облік за estimated falling у [start,end), closing/drain/barrier у чинних бюджетах03, пізні належні samples враховуються; неповний/неоднозначний доказ не дає pass. |

**B — внесені виправлення:**

- CHK005: normal Disabled лише за справного стану; fault видимий як Fault/LOW.
- CHK010: серіалізація/recheck перед усіма nonzero API, включно зі stop idle1.
- CHK013: overflow recorder не блокує LOW/fallback і не викликає рекурсивне журналювання.
- CHK014: checked LOW перед accuracy stop; API failure не стає pass, LowUnconfirmed блокує normal freeze/stop.
- CHK016:112мс до сталого terminalLOW із10с guard; звичайна LOW-фаза PWM не є cutoff.
- CHK020: edge-only CSV потребує initial levels, acquisition bounds, observed windows і gap/overflow metadata.
- CHK025: стабільні wire enum codes та обов’язковий explicit accuracy trace-step без default.
- CHK027: raw status кожної API operation у records, first/fallback header — додатковий summary із presence semantics.
- CHK033: M04 позначено визначенимD60; фізичні gates залишено відкритими.

**A — вже задоволено:** CHK001, CHK002, CHK003, CHK004, CHK006, CHK007, CHK008, CHK009, CHK011, CHK012, CHK015, CHK017, CHK018, CHK019, CHK021, CHK022, CHK026, CHK028, CHK029, CHK030, CHK031, CHK032. Докази й посилання на фактичний текст наведено в таблиці розбору вище; номери рядків у ній відповідають моменту первинного розбору до D61.

**Змінені документи:** spec.md, plan.md, research.md, data-model.md, contracts/pump-output.md, quickstart.md, цей checklist; нове рішенняD61 у docs/decisions.md. Історичні ADR01–60, конституцію та попередні features не переписано.

**Перевірки:**33 унікальні пункти/33 позначки[x], traceability33/33; локальні посилання й git diff --check; повторно звірено межі RAM/формату/часу. Extension hooks не зареєстровані (.specify/extensions.yml відсутній). Analyze не викликався, зовнішній analyze report відсутній. Нові firmware host/target/physical перевірки не виконувались; їхній майбутній scope доповнено fixtures для stress-window evidence.

**Відкриті пункти checklist:** немає. **Фізичні gates:** Q02/H02/H03, Q03 conditioning/capture evidence, Q06 MD20A revision/interface/jumper залишаються відкритими. Q05/силовий тракт/напрямок насоса належать05/06.

**Наступний крок:** окремий явний виклик /speckit-tasks для04. Tasks/Implement не запускались.


### Уточнення після analyze-guided, D62

2026-10-30 власник обрав A для U1: замість первісного 600 MCU-секундного RAM window D61 чинні 601 MCU-секунда [t0+5 с,t0+606 с). Output schedule лишається D60 з LOW на t0+605 с; collector/max diagnostics тривають до 606 с, export чекає closure/freeze. Окреме фізичне прийняття вимагає≥600 с покриття зневизначеністю за контрактом. Історичні цитати/номери рядків і 33/33 вище належать моменту первісного checklist review, а не новим hardware доказам. Рішення щодо bounded RAM/capture continues збережено. Повний аналіз і виправлення — [analysis-resolution.md](../analysis-resolution.md).
