# План реалізації: основа й апаратний профіль

**Ітерація / гілка**: `01-foundation`

**Дата**: 2026-10-28

**Специфікація**: [spec.md](spec.md), включно з трьома відповідями Clarify.

**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/01-foundation`.

**Статус**: Phase 0/1 design підготовлено; реалізація й прийняття не виконані.
Q01 має вибрані версії, але E01 bootstrap-lock ще не пройдено. За D18–D20
контактний дизайн RC=GPIO3/PWM=GPIO0 із DIR–GND на MD20A за D20 і живлення 3,3 В визначено; додаткова схема/continuity
не блокує програмну підготовку. Фізичні дії/прийняття залежать від H02–H05.
Після Plan окрема [ROM/USB-сесія](verification/board-review.md) підтвердила
chip/revision/flash/USB частину H01; GPIO/електричне прийняття не виконане.

## Підсумок

Майбутня основа — native ESP-IDF проєкт у `firmware/`, переносимий C++ startup
supervisor і платформні адаптери. У 01 немає RC capture, робочого PWM, arming або
керування насосом. Дозвіл роботи завжди false; PWM утримується LOW на погодженому
контакті SuperMini. DIR апаратно з’єднується з GND на MD20A за D20; GPIO1 не використовується. MD20A та насос від'єднані за D11.

Після init failure — автоматичні bounded retries за D12: 5 спроб загалом,
1 000 000 мкс від завершення невдалої спроби. Успіх веде в ReadyDisabled, п'ята
невдача — в Exhausted без application auto reset. Діагностика працює поза init path.
Мінімальний supervisor не є повним control core 02 або operational failsafe 05.

## Технічний контекст

| Поле | Вибір і стан |
|---|---|
| Мови | strict C++11 для bridge_core, C++17 для app/platform; без Arduino framework. |
| SDK | ESP-IDF v5.5.6, commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`. |
| Target toolchain | riscv32-esp-elf `esp-14.2.0_20260121` із tagged tools.json. |
| Build tools | CMake 3.30.2, Ninja 1.12.1, CPython 3.11.7; archive pins у research. |
| Host compiler | Apple clang 21.0.0 (`clang-2100.1.1.101`), CLTools 26.6.0.0.1781586589, SDK 26.5, macOS arm64. |
| Python closure | E01: full exact-version/hash lock і wheelhouse створюються/відтворюються до залежної target-реалізації/збірки; зараз їх немає. Підготовка самого bootstrap дозволена для закриття E01. |
| Flash build settings | Mode/frequency/partition layout ще не вибрані; власники Q01/Q02 у 01, до залежного target configure за первинними SDK/апаратними джерелами. Без мовчазних defaults. |
| Тестування | Standalone C++ executable + CTest, explicit checks; окремий negative probe; жодного стороннього test framework. |
| Платформа | ESP32-C3 SuperMini, профіль c3-supermini-pinout-01 для одиниці c3-supermini-188b0e93032c; дизайн за D18–D20, фізичне прийняття окремо. |
| Контактний дизайн | За D18–D20: RC=GPIO3, PWM=GPIO0, DIR–GND на MD20A без GPIO1; зовнішні 3,3 В на 3V3/GND. Профіль погоджено для програмної підготовки, не електричного прийняття. |
| Зберігання | Volatile startup state; без NVS, learned endpoints і persisted permission. |
| Runtime час | Монотонні uint64 мкс поточного boot; адаптер esp_timer, scheduler poll 10 мс. |
| Діагностика | USB Serial/JTAG; static snapshot mailbox, окремий consumer poll 100 мс, лише нові sequence одноразово за D22; без heartbeat/replay після drop/reconnect. |
| Масштаб | Один майбутній pump channel; R01,R02,R08,R10,R12. 07–09 не є targets. |
| Output | У 01 лише статичний PWM LOW; DIR–GND на MD20A, програмного DIR немає. Погоджені 1000–2000 мкс/1000 Гц — інваріанти наступних етапів, не функції цього каркаса. |

SDK availability, C3 RMT/LEDC і конкретні tools pins перевірено в первинних джерелах;
див. [research.md](research.md). Це не цільова збірка. Наявний Python/Clang inventory
не означає, що SDK/CMake/Ninja встановлені або середовище вже відтворено.

### Рішення та відкриті execution gates

| Gate / власник | Передумова та вплив | Поточний стан |
|---|---|---|
| E01 / Q01, 01 | SDK/submodules/tools hashes, exact Python closure, clean hash-locked replay, host/version checks; без цього target build не є відтворюваним доказом | Blocked: bootstrap не виконувався; дизайн pin/lock protocol визначено |
| H01 / Q02, 01 | Profile ID, chip/flash, погоджений користувачем pinout, GPIO/SoC conflicts, design source fingerprint | Pass design за D17–D20: c3-supermini-pinout-01, RC=GPIO3/PWM=GPIO0, DIR–GND на MD20A за D20, 3V3/GND; окрема схема/continuity не потрібна для коду/збірки |
| H02 / Q02/Q06, 01 | Для обраних 3,3 В перевірені фактичні напруги/живлення та зовнішній bias, no-load стенд | Blocked тільки фізичне прийняття; 3V3/GND як спосіб живлення вже обрано |
| H03 / Q02/Q06, 01 | До вимірювання погоджені числові електричні межі, прилад, resolution, trigger, тривалість/повтори | Blocked: методика потребує фактичного профілю/обладнання; SC-006 відкритий |
| H04 / Q03, 01/03 | RC levels/conditioning до підключення; source timing/loss до capture acceptance 03 | Blocked для RC-підключення; RC не потрібний для обов'язкових GPIO-only спостережень 01 |
| H05 / Q06, 01/04/06 | У 01 документальна основа MD20A interface/protection; actual driver interface до 04, power/stall/transients/thermal/emergency cutoff до 06 | Фактичні параметри не підтверджені; MD20A та насос у 01 від'єднані |

За рішенням D21 (CHK007, A) SC-004 у 01 приймає SuperMini та її застосовні
електричні поля; H02/H03 і SC-006 залишаються обов'язковими. RC/MD20A у 01 мають
документальні вимоги, статуси й gates. Фактичні дані RC належать 03 до підключення,
ревізія/інтерфейс/перемичка MD20A — 04 без насоса, силовий тракт — 06; вони не
блокують SC-004 у 01 і не видаються за прийняті.

Ці gates не закриваються назвою SuperMini, успішним host-тестом чи припущенням.
Програмний дизайн supervisor/контрактів та підготовка визначеного GPIO-профілю не
потребують повторної апаратної схеми за D18. Їхня незалежна
частина не блокується відкладеними Q04–Q09; залежна апаратна робота не починається
до свого gate. Пороги/RC timeout/arming/watchdog operational policy залишаються Q04/Q05.

## Перевірка конституції

Конституція v1.0.0 заповнена й ухвалена 2026-10-28. D10 дозволяє feature-гілки
під час Specify; поточне пряме завдання Plan забороняє будь-які branch/commit дії,
тому гілку не змінювали. Foundation історичний і не визначає GPIO/платформу.

| Принцип | До дослідження | Після дизайну |
|---|---|---|
| I — Scope | Pass: одна pump-основа, 07–09 поза обсягом | Pass: немає operational control, radio або нових targets |
| II — Safety | Pass для незалежного дизайну; hardware частина blocked H02/H03/H05 | Pass контрактів: LOW-first, retries без permission, diagnostics isolation; фізичний доказ blocked, не waived |
| III — Architecture | Pass меж core/platform; на момент початкового research профіль був відкритий | Pass: D18–D20 визначили контактний дизайн; H02/H03 залишаються фізичним прийняттям, не блоком software design |
| IV — Evidence | Pass owned Q01 research gate; build/physical не заявлені | Pass дизайну pins/методів; Blocked відтворення E01, SC acceptance не пройдено |
| V — Workflow | Pass: явний каталог, лише Plan, чинні файли збережено | Pass: тільки design-артефакти й запис рішень, без Tasks/Implement/Git/hardware дій |
| VI — Decisions | Pass: D09,D11–D13 та Q owners прочитано | Pass: D14–D22 записано; E01/H02–H05 відкриті, H01 design визначено за погодженим pinout; межу SC-004 уточнено D21, одноразові diagnostic events — D22 |

Blocked hardware/environment gates зупиняють **відповідну** реалізацію/прийняття,
а не перетворюються на Pass. Таблиця не є дозволом обійти конституцію. Загальних
web/API, обов'язкових test-first approval циклів чи додаткових функцій не додано.

### Перевірка й простежуваність

| Вимоги | Спосіб реалізації/перевірки в майбутньому | Критерії та запис |
|---|---|---|
| R01 / FR-001 | Static LOW-only composition; перевірка відсутності RC/arming/LEDC path | SC-002,SC-006; build.md,hardware.md |
| R02 / FR-002,FR-003 | Pinned SDK/tools, E01, build wrapper без flash | SC-001,SC-002; build.md |
| R01,R02,R08 / FR-004,FR-005,FR-006 | Profile schema/preflight, H01/H02/H04/H05, chip/pin/electrical review | SC-004; board-review.md |
| R08 / FR-007,FR-008 | Startup supervisor + LOW-first adapter; retry deadlines/ordering | SC-003,SC-006; host.md,hardware.md |
| R10 / FR-009 | Minimal component graph + generated config/map/startup review | SC-005; config-review.md |
| R02 / FR-010 | Окремий core target strict C++11 без SDK headers | SC-003; host.md |
| R08,R12 / FR-011 | State/deadline/fault/mock-order suite + negative probe | SC-003; host.md |
| R08 / FR-012,FR-013 | GPIO-only bench, зовнішній bias, окремі startup-context owners | SC-004,SC-006,SC-007; board-review.md,hardware.md |
| R12 / FR-014,FR-015 | Environment/source fingerprints, separate evidence kinds, unperformed states | SC-008; усі verification records |
| R02,R08,R12 / FR-016 | Review E01/H-gates та узгоджених рішень до залежної роботи | SC-001,SC-004,SC-008; build.md,board-review.md |

Шляхи записів тут відносні до майбутнього `specs/01-foundation/verification/`.
Посилання на T-IDs додаються лише окремою фазою Tasks; самих tasks.md/тестів/доказів
цей Plan не створює. Деталі перевірок: [build-and-evidence.md](contracts/build-and-evidence.md).

## Структура проєкту

### Документація ітерації

```text
specs/01-foundation/
├── spec.md
├── checklists/requirements.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
└── contracts/
    ├── startup.md
    ├── board-profile.md
    └── build-and-evidence.md
```

### Майбутній код — не створено під час Plan

```text
firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
│   ├── CMakeLists.txt
│   └── app_main.cpp
├── components/
│   ├── bridge_core/
│   │   ├── CMakeLists.txt
│   │   ├── include/bridge/startup.hpp
│   │   └── startup.cpp
│   └── platform_esp32/
│       ├── CMakeLists.txt
│       ├── include/platform/startup_port.hpp
│       ├── gpio_safe_output.cpp
│       ├── monotonic_clock.cpp
│       └── local_diagnostics.cpp
└── boards/                     # Матеріалізація погодженого профілю D18–D20
tests/host/
├── CMakeLists.txt
├── startup_tests.cpp
└── negative_probe.cpp
scripts/
├── build.py                    # host/target wrapper, не flash
├── environment/check.py
└── verify/config.py
tools/environment/              # SDK/tools/Python lock і hashes після E01
```

`firmware` — ESP-IDF root. `main` складає адаптери й supervisor; core не включає
SDK/GPIO/FreeRTOS. Generated profile header та sdkconfig розміщуються в окремому
build directory. JSON parser потрібний лише host preflight, не firmware.
Реальні папки зараз мають README; дерево вище — дизайн, не доказ реалізації.

## Послідовність дизайну реалізації

1. Підготувати й перевірити зафіксоване середовище E01; не оголошувати його готовим
   до replay exact lock. Host-only робота потребує своїх підтверджених compiler/tools.
   Підготовка bootstrap/lock tooling для закриття E01 дозволена; залежний target
   firmware починається після E01. До target configure також записати flash mode,
   frequency та partition layout з джерелами; невідомі settings блокують цей крок.
2. Окремо реалізувати portable startup state/actions та meaningful host suite;
   не створювати повний control core 02 або hardware stub, який видається за плату.
3. Матеріалізувати визначений у [контактному профілі](../../hardware/esp32-c3-supermini-profile.md)
   H01 design у майбутній JSON/config без повторного запиту схеми. Main перевіряє compiled profile;
   до дозволеного GPIO доступу не запускає console/init повідомлень.
4. За профілем встановити software PWM LOW; DIR GPIO-операцій немає; помилка assert LOW
   припиняє інші init-кроки. Кожна повторна спроба повторює цю безпечну послідовність.
5. Запускати retries і відокремлений diagnostic consumer. ReadyDisabled/Exhausted
   залишають permission false. Core clock regression/protocol faults завершують
   session без дозволу; відновлення — новий reset, не прихований auto reboot.
   За D22 consumer передає лише новий sequence одноразово; drop/reconnect не
   повторює збережений snapshot. Поточний/термінальний стан після втрати запису
   може не бути видимим до нової події чи окремо дозволеного reset.
6. Виконати clean target build/config review без flash. Після окремого дозволу та
   H02/H03 — GPIO-only спостереження; SC-006 не замінюється software evidence.

Це опис порядку, не згенерований список Tasks і не запуск Implement.

## Облік складності

| Компроміс | Навіщо | Чому простіша альтернатива не підходить |
|---|---|---|
| Окремий portable supervisor | Host перевірка retry/time/fault invariants | Логіка в app_main зв'язала б поведінку з SDK |
| Compile-time перевірений board profile | Не допустити guessed pins та unsupported board claims | Назва SuperMini не визначає розводку/bias |
| Окремий diagnostic consumer | Console backpressure не затримує startup | Синхронний stdout може чекати host |
| E01 і H-gates | Розрізняти вибір дизайну й фактичні докази | «Збирається» не доводить hardware safety або dependency replay |

Винятків із конституції для реалізації немає; відкриті gates не виправдовуються
складністю. Наступна можлива фаза — Tasks лише за окремим запитом після review
цього плану й визначення роботи, дозволеної відкритими gates.
