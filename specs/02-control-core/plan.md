# План реалізації: переносиме ядро керування

**Ітерація**: `02-control-core` | **Дата**: 2026-10-28 | **Специфікація**: [spec.md](spec.md)  
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/02-control-core`  
**Стан**: план; реалізація і прийняття не виконані.

## Summary

Окреме стандартне C++ ядро для одного каналу перевіряє ширину до обмеження, обчислює фіксовану карту цілими відсотками та зберігає розрахунок окремо від фактичної команди. У 02 фактична команда завжди 0%, production arming відсутнє. Capture fault фіксується; подальший valid може оновити розрахунок, але не дозволяє вихід. Дизайн, інтерфейс і перевірки — у [research.md](research.md), [data-model.md](data-model.md), [контракті](contracts/control-core.md) і [quickstart.md](quickstart.md).

## Technical Context

- **Мова та залежності**: strict C++11, без RTTI/exceptions за D14; core без SDK, GPIO, FreeRTOS, I/O, динамічних платформних залежностей чи heap. Host: Apple clang 21, macOS arm64, CMake 3.30.2, Ninja 1.12.1, Python 3.11.7; target: ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, riscv32 `esp-14.2.0_20260121`. Хеші й локальні шляхи — `tools/environment/environment.lock.json`, повторна перевірка обов'язкова; тестовий harness 01/CTest, без нового framework.
- **Зберігання**: лише стан у пам'яті; endpoint learning і постійне зберігання відсутні.
- **Перевірка**: host без плати/заголовків SDK, окремий target build без flash; незалежні physical records. Fake monotonic clock передає час явно, без системного годинника у ядрі.
- **Ціль**: host і native ESP32-C3 SuperMini, `c3-supermini-pinout-01` — погоджений design, не фізично прийнятий стенд. Інші плати не заявлено.
- **Час/продуктивність**: PWM 1000 Гц і його вимірювання належать 04; числовий timeout/cutoff та operational recovery — Q05/05. У 02 немає припущеної межі затримки чи таймера відмови.
- **Межі**: карта 1000–2000 → 0–100%, валідність 900–2100 включно, без фільтра та автокалібрування; Wi-Fi/Bluetooth не вмикаються. Лише один pump input; 03–09 і робочий насос поза 02.
- **Рішення**: D36/D37 закривають поведінку Q04, потрібну 02; період, виявлення втрати/overflow та фізичні похибки — 03. Q03 до підключення у 03; Q02/Q06 — фізичні gates; Q05 до operational behavior 05; Q07–Q09 відкладені. 100 мс — історична пропозиція, не рішення.
- **Залежність 01**: [програмні host/build результати](../01-foundation/verification/acceptance.md) існують; 01 не прийнята повністю, H02/H03 і SC-004/SC-006 відкриті. За D38 завершення 02 можливе після виконання її критеріїв і перевірки потрібної програмної основи 01 без повного фізичного прийняття 01. H02/H03 та залежне фізичне прийняття залишаються відкритими; це не дозвіл роботи з насосом.

## Constitution Check

Конституція v1.0.0 заповнена й ухвалена 2026-10-28, не шаблон. До research і після дизайну перевірено окремо; «pass» стосується зазначеного обсягу планування, а не фактичного прийняття обладнання.

| Gate | До research | Після дизайну |
|---|---|---|
| I — Scope | Pass: один канал, D07, 07–09 поза обсягом. | Pass: контракт без радіо/додаткових каналів; PWM 04. |
| II — Safety | Pass для плану: D36/D37, Q05 відкладено, фізичні gates відкриті. | Pass для portable scope: disarmed/fault 0%, без arming; bias та незалежний cutoff окремо, LOW MD20A — brake. Робоче прийняття blocked до 05/06. |
| III — Architecture | Pass: D02/D14, явні межі. | Pass: C++11 ядро без I/O та платформних include; профіль design-only, compatibility не заявлена. |
| IV — Evidence | Pass для плану: pinned 01 і окремі види доказів. | Pass для плану: матриця нижче; фактичні host/build/physical 02 не виконано, H02/H03 01 blocked. |
| V — Workflow | Pass: лише Plan, заданий каталог, жодних Git/hardware дій. | Pass: setup повернув шляхи 02; D38 дозволяє завершити 02 без повного фізичного прийняття 01; залежне фізичне прийняття відкрите. |
| VI — Decisions | Pass: Q04 D36/D37, Q05/03/06 мають власників. | Pass: відсутні приховані timeout/rearm/електричні defaults; поведінка уточнена в контракті. |

### Verification and Traceability

Це план майбутніх тестів, не результати; завдання генеруються лише окремою фазою.

| R / FR | Сценарій / критерій | План перевірки | Доказ, статус |
|---|---|---|---|
| R03 / FR-001,006 | US3, SC-003/004 | Host strict C++11 isolated header/link probe без SDK і перехресна перевірка залежностей; дисарм і clear. | `verification/host.md` — not_run |
| R04 / FR-002,004 | US1, SC-001 | Шість точок, 900/999/1000/1001/1999/2000/2001/2100, кожна межа округлення ±1 мкс; без learning. | `verification/host.md` — not_run |
| R05 / FR-003,005,006 | US2/3, SC-002/003 | 899/900/2100/2101, великі значення, invalid-to-high, без попереднього valid, invalid після valid, capture fault і valid під fault, clear і high; last-valid time не оновлюється від invalid. | `verification/host.md` — not_run |
| R12 / FR-007 | SC-004 | Окремі host, native target build/config review та фізичний запис із умовами й статусом; без підключення. | `verification/host.md`, `build.md`, `hardware.md` — not_run |

## Project Structure

### Documentation

`spec.md`, `plan.md`, `research.md`, `data-model.md`, `contracts/control-core.md`, `quickstart.md` — артефакти 02. `tasks.md` і `verification/` — майбутні, не створюються плануванням.

### Source Code

Наявні `firmware/components/bridge_core/startup.cpp`, `include/bridge/startup.hpp`, `tests/host/` і `scripts/build.py`. Заплановано окремі `firmware/components/bridge_core/control.cpp`, `include/bridge/control.hpp`, `tests/host/control_tests.cpp`; інтегрувати новий source у наявні host та target CMake, без зміни апаратного output на operational. Не впроваджувати платформи або capture у core. Назви — дизайн для наступної фази, не доказ існування.

## Complexity Tracking

Розділення збереженого розрахунку й фактичної команди необхідне за D36/D37: одне поле не може одночасно зберігати 80% і забезпечувати 0% при fault. Додатковий arming state чи timeout у 02 не додається.
