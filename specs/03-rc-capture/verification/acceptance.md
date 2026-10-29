# Виконання 03-rc-capture

2026-10-29. Програмна реалізація дозволена користувачем із відкритим фізичним checklist. H02/H03/Q03 і T043–T047/T051 залишаються відкритими. Без flash/load/commit/push.

Початковий HEAD: 027f89f367027582496db973ebbd7c7e7b4056a1

~~~text
 M docs/architecture.md
 M docs/decisions.md
 M docs/open-questions.md
 M docs/requirements.md
 M docs/roadmap.md
 M hardware/README.md
?? "hardware/\321\201\320\265\321\200\320\262\320\276-\321\202\320\265\321\201\321\202\320\265\321\200.pdf"
?? specs/03-rc-capture/
~~~

Початкові source hashes — initial-source-hashes.json. Це вихідний стан, не результат перевірок.

## Підсумок програмного виконання

**Програмну частину завершено; ітерацію03 фізично не прийнято.** 48/54 tasks виконано. T043–T047 і T051 залишено unchecked: немає електричних/приладних доказів і вимірювань. Поточне явне доручення користувача дозволяє code/host/target builds із цими задачами відкритими, тому повторного підтвердження не запитували.

Реалізовано один input GPIO3: modern native RMT width + estimated GPIO rising/falling, coherent epoch/sequence association, width/period validation, last-valid=end тільки valid; portable state/core збережено disarmed0. Faults/barrier/resync/guards і100ms HIGH/LOW monitor не очищують core latch. Diagnostics bounded/latest-only/zero-wait,10с, one-in-flight; bench stress і128-record RAM collector/frozen export/offline comparator. Capture API стандартного C++11 без IDF/FreeRTOS/I/O/heap; ControlCore02 API/код не змінено. GPIO0LOW, permissionfalse, DIR GPIO немає; output PWM/production arm/clear/operational timeout не додано.

| Evidence | Фактичний результат |
|---|---|
| [environment.md](environment.md) | Pinned environment gate pass, версії01 не змінено |
| [host.md](host.md) |18/18 CTest,17/17 tooling pass, negative probe exit1 як очікується |
| [build.md](build.md) | normal/stress/accuracy native C3 builds pass |
| [config-review.md](config-review.md) |3 config/graph verifications pass; IRAM/ROM direct closure42 functions + розв’язані indirect callbacks/data review |
| [software-evidence.json](software-evidence.json) |61 source/input hashes,50 build/test artifact hashes, HEAD/branch/Python |
| [hardware-procedure.md](hardware-procedure.md) | Підготовлена ручна процедура, не виконання |
| [hardware.md](hardware.md) | not_run/blocked: H02/H03/Q03, U/прилад, timing/accuracy/loss/backpressure |

Повідомлення build tool із flash command є довідкою. У цьому виконанні flash/monitor/device I/O/load, Git branch mutation/stage/commit/push не виконувались. Початкові user changes вище збережені, prompt inputs і історичні01/02 records не переписано. Existing ignored build directories не видалялися. Фінальні outputs знаходяться у specs/03-rc-capture; prereq workflow з явно заданим SPECIFY_FEATURE_DIRECTORY повернув саме абсолютний шлях03.

## R / FR / SC → task → evidence

| Вимоги / критерії | Tasks | Software evidence | Відкритий physical evidence |
|---|---|---|---|
| R05,R09 / FR-001,002 / SC-001 | T004–T018 | capture validation/association/core suites,6map points/endpoints/rounding,periodorigin/last-valid/invalid-clamp; host.md | T045: actual HJ timing/accuracy |
| R05,R09 / FR-003,006 / SC-001,002 | T019–T030 | fault/time/adapter suites; ended episode/masking/barrier/retry, output0/latch; host.md | T044,T045,T047: physical LOW,100..110ms, rearm/processing/stress |
| R09,R11 / FR-004 / SC-003,004 | T031–T039 | diagnostics policy/transport, bounded format/admission/accounting; host.md, config-review.md | T046,T047: USB backpressure/partial/reconnect, max-load600с |
| R12 / FR-005 / SC-003,004 | T001–T003,T007,T018,T029,T039–T041,T049,T050 | environment/host/tooling/3builds/config graph/IRAM/data/hashes | T043–T047,T051: E≤5/10us each,U≤1us separately,0loss600с |
| R12 / FR-007 / SC-004 | T003,T042,T048–T050 | prerequisites.md/hardware procedure/current pending states | T043 gate H02/H03/Q03, receiver gate до використання |
| D53,D54,D55 / SC-003,004 | T052–T054,T038–T042 | bounded128records/freeze/export, Decimal comparator directE/U, cross-session/partial failures; host/tooling/build | T045 короткі windows, T046 окремі600с counts/timing; обидва not_run |

SC-001 програмно pass; SC-002 fake-clock/state pass, runtime timing/LOW pending. SC-003 фізично not_run, host/build її не закривають. SC-004 evidence/traceability ведеться з явними невиконаними перевірками. Capture-boundaries checklist31/31; requirements10/11, physical item відкритий.

## Constitution review

| Принцип | Оцінка поточної роботи |
|---|---|
| I Scope | pass: один input/nativeIDF/50Hz, карта02 без learning/filtering; radio/другий канал/Arduino не додано |
| II Safety | software pass: zero/disarmed/latch/monitor, без output arm; physical suitability blocked H02/H03/Q03, operational cutoff/watchdogQ05→05, powerQ06→04/06 |
| III Architecture | pass: portable C++11 DTO/policy, C++17 adapters, profileGPIO3/0, жодних нових pins/платформ |
| IV Evidence | pass для software: pinned builds, meaningful positive/boundary/fault tests, hashes; повне hardware acceptance pending |
| V Workflow | pass: лише явно доручений Implement, SPECIFY_FEATURE_DIRECTORY03, українські docs, user changes/prompt inputs збережені; Git/hardware actions не виконано |
| VI Decisions | pass: D51–D55 відокремлені від measured results; legacy hardware latch/3ms замінені явнимD51; Q03/Q05/Q06 та receiver owners збережені |

Конституція v1.0.0 заповнена й ухвалена2026-10-28, не placeholder. PROJECT_FOUNDATION.md лишається історією; WROOM/двоканальний старт і100ms operational timeout не стали defaults. Поріг100ms у03 стосується capture stuck, не Q05.

Extension hooks до/після Implement: .specify/extensions.yml відсутній, dispatch не потрібний. Наступну фазу/ітерацію автоматично не запускали. T051 залишається blocked до фактичних physical results; цієї сесії недостатньо для твердження, що03 завершена за roadmap.

## Програмне доповнення D56

За окремим прямим дорученням користувача firmware вимикає внутрішні pull-up/pull-down GPIO3 після створення RMT RX, до enable. Зовнішня схема задає bias; запланований дільник HJ5,6/10кОм не позначено фізично прийнятим. API failure має cleanup/fault/retry.

Повторно: host18/18, tooling17/17 pass; три нові clean target builds build/c3-03-bias-normal/stress/trace і config checks pass; actual ELF підтверджує GPIO3/FLOATING/порядок/checked return. Поточні hashes у software-evidence.json, попередній запис та докази у gpio-bias-before/, старі build directories збережені. Після цієї зміни для прошивання треба використовувати нові bias builds.

Фізичні T043–T047/T051 відкриті; кількість48/54 не змінена. Flash/monitor/load/Git mutations не виконувались. Програмне вимкнення підтяжок не є підтвердженням допустимої напруги HJ або роботи дільника.

## Виправлення USB RX після boot-log користувача

Користувач навів коректну команду PlatformIO monitor і ROM boot log без application rc03. Read-only аналіз pinned SDK виявив помилку нашої конфігурації: RX64 не задовольняє strict >64, тому USB diagnostic driver не встановлюється. Виправлено RX128 зі спільною testable константою; TX512, no commands, capture/core/arming/D56 без змін.

Після виправлення: host18/18,tooling17/17, три нові clean build/c3-03-usb-normal/stress/trace і config checks pass. Негативний isolated RX64 experiment дає expected exit1. Поточні source/artifact hashes — software-evidence.json; попередні records — usb-rx-before/, старі bias binaries збережено. Поточний host build оновлено інкрементально.

Агент не відкривав serial port, не прошивав/не перезапускав плату. Наведений користувачем boot log не є повним фізичним прийняттям; оновлена прошивка на платі ще не перевірена. T043–T047/T051 лишаються відкритими;48/54 tasks. Для наступного ручного flash потрібна нова usb build, не попередня bias build.

Оператор згодом надав rc03 output: USB application diagnostics спостерігається, GPIO3 HIGH/no complete pulse/StuckHigh0x20; recorded lateness161мкс у software clock. Деталі/межі — [operator-usb-log-2026-10-29.md](operator-usb-log-2026-10-29.md). Повне фізичне прийняття лишається відкритим, підключення входу уточнюється.

Часткове операторське спостереження HJ1701мкс описано у [operator-hj-1701-log-2026-10-29.md](operator-hj-1701-log-2026-10-29.md): latest valid1702–1704мкс,duty70/effective0; історичні invalid1120 не ростуть у фрагменті. Без analyzer/U та підтверджених conditions hardware acceptance не змінено; історія reset/invalid уточнюється.

Історія HJ1701 уточнена: користувач підтвердив reset, інших width/frequency не задавав. Скидання f20→0 пояснено reset;1120invalid залишається невстановленою причиною, не оголошено нормальним startup. Подробиці у operator-hj-1701-log-2026-10-29.md; фізичне прийняття не змінено.

Нове явно дозволене read-only monitoring поточного HJ sweep800–2200 виконано60с: [звіт/raw/analysis](serial-sweep-20261029T155006Z-report.md). Width802–2203,invalid нижче900/вище2100,valid усередині; видимих status/map mismatches0. Source counters у continuous tail+2996(candidates)=2396valid+600invalid,detected drop/fault/duplicates0. Один stale committed record збережений та відокремлений від нового interval; точність/zero-loss600с і формальне acceptance не підтверджено. Тести/пороги/firmware/навантаження не змінювалися; T043–T047/T051 відкриті.
