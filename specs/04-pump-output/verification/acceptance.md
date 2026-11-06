# Проміжне прийняття04-pump-output

2026-10-30. **Програмні задачі виконано; повне прийняття04 відкрите.** Фінальний host:22/22 CTest,28/28 tooling; negative probe expected exit1. Pinned native C3 builds9/9, config9/9; ELF ISR review43 IRAM/ROM functions у кожному образі. Фізичні T044–T050 не виконано, Q02/H02/H03, Q03 і Q06 залишаються відкритими. Завершення всієї ітерації не заявляється.

Реалізовано portable finite output policy, native LEDC adapter, окремі waveform/stress/accuracy images, єдиний output owner, terminal fault→LOW із checked fallback, bounded RAM out/win/accuracy evidence, rc04 diagnostics та offline validator. Normal startup/RC/USB не дозволяють operational output; Q05 arming не реалізовано. Actual GPIO/timing/напругу/MD20A з software не виводимо.

Докази: [host](host.md), [build](build.md), [config/ELF](config-review.md), [hardware/open gates](hardware.md), [точні hashes](software-evidence.json), [tasks](../tasks.md), [методика/CLI](../quickstart.md). `pass software` нижче означає fixtures/build/static review, не фізичний pass. [Spec](../spec.md), D59–D62 та M04 залишаються нормативними; попередні записи01–03 не переписано як вимірювання04.

## FR / R / US / SC → фактичний стан

| FR / requirements | Сценарії | Software evidence | Фізичний стан / наступний доказ |
|---|---|---|---|
| FR-001 / R01,R06 | US2.AC5 / SC-003,006 | GPIO0/no DIR/profile contract; adapter/config pass | blocked Q06; T045 jumper/interface, T047 GPIO0 і MD20A input |
| FR-002 / R04,R06 | US2.AC1–4 / SC-001–004 | count/endpoints/invalid request, production SDK transcript та comparator fixtures pass | not_run T047,T049;1000Гц/six points/static endpoints/voltage/transition M04 |
| FR-003 / R08 | US1.AC1–2 / SC-003,004 | normal/disarmed permissionfalse/effective0, startup LOW та default-off build pass | not_run T046, Q02/H02/H03 до firmware |
| FR-004 / R01,R08,R11 | US1.AC1–4 / SC-004 | initialLOW5с, finite65/605с, no-repeat, reset reason/startup retry/deadline boundaries pass | not_run T046–T049; reset кожного типу й no-repeat фізично |
| FR-005 / R06,R08 | US3.AC1–2 / SC-004 | API/fallback/stale/fault ordering/sticky first budget/LowUnconfirmed fixtures pass | not_run T048; прямі GPIO3→GPIO0≤112мс +guard, MCU API brackets окремо |
| FR-006 / R04,R05,R08 | US3.AC2–3 / SC-001,004,005 | 02/03 validation-before-clamp/mapping/invalid hold/last-valid/latch/resync regression pass | not_run T050; capture physical accuracy03 залишається відкритою |
| FR-007 / R08,R09 | US3.AC2–4 / SC-004,005 | finite owner/scheduler, fault/capture fixtures, RAM closure/budgets та config pass | not_run T048–T050; runtime latency/loss/accuracy не виміряно |
| FR-008 / R11,R09 | US3.AC4 / SC-005 | coherent calculated/effective/request/applied/permission fields;383/384B, slots/drop/cursor/reconnect/immutable archives pass | not_run T049; active/slow/absent reader stress та stack high-water |
| FR-009 / R08,R12 | US3.AC5–6 / SC-003,006 | boot policy tested; [методика](hardware-procedure.md) оформлена | blocked Q02/Q03/Q06; T046; brownout/watchdog full gates05/06 |
| FR-010 / R01,R08,R12 | US2.AC5,US3.AC6 / SC-003,006 | design profile/physical evidence boundaries явно збережені | blocked T044–T045; фактичні unit/revision/wiring/no-pump readiness |
| FR-011 / R12 | US3.AC6 / SC-002–006 | M04/schema/Decimal/digest/negative fixtures/exit0,1,2 pass; методику не послаблено | not_run T044,T046–T050; придатність приладу/U/provenance до запуску |
| FR-012 / R02,R03,R10,R12 | US1.AC1,US3.AC4 / SC-001,005,006 | fresh22 host +28 tooling, pinned9 target/config, C++11/17/native graph/IRAM pass | build не приймає hardware; T044–T050 open |
| FR-013 / R12 | US3.AC6 / SC-006 | ця matrix, actual commands/hashes/logs/statuses/owners; user work збережено | повне прийняття blocked до всіх обов’язкових фізичних evidence |

## Критерії успіху

| SC | Стан |
|---|---|
| SC-001 | **pass software**: шість nominal values, rounding/invalid/fault/no-permission перевірені actual core/adapter fixtures |
| SC-002 | **not_run**: потрібні per-cycle frequency/duty двох точок вимірювання, transitions та M04 U; comparator fixtures не є приладом |
| SC-003 | **blocked/not_run**: static levels/startup/reset/bootloader/upload/jumper фізично не прийняті |
| SC-004 | **pass software; physical not_run**: finite/no-repeat/reset/fault/API-failure/LOW-before-stop covered; потрібні T046–T048 |
| SC-005 | **pass software; physical not_run**: collector/late processing/601MCU/600physical lower-bound/parser/diagnostics covered synthetic; T049–T050 лишаються відкритими |
| SC-006 | Простежуваність усіх13 FR оформлено; **full acceptance blocked** через7 операторських задач, без непроведених pass |

US1–US3 мають software evidence; жодна історія не оголошується фізично прийнятою. D62:601 MCU-секунда та≥600 фізичних секунд — окремі докази; clock-offset підгонки немає. Accuracy128-record windows не доводять повний65с/600с run. MCU API успіх не означає виміряний LOW. PWM LOW MD20A — brake; coast/power isolation/нуль витрати не заявляються.

## Конституційний review

| Принцип | Результат і межа |
|---|---|
| I — Bounded Local Pump Control | Один RC input/GPIO3, PWM GPIO0/native C3/MD20A; DIR hardwired; без Arduino/radio/ignition/reversal/operational arming |
| II — Explicit Safety and Fail-Off | Normal LOW/disarmed; bench із окремого образу, finite/fault latch/checked LOW. Фізичні bias/reset/driver та незалежне силове відключення не підмінено програмним результатом |
| III — Portable Core and Verified Boundaries | Strict C++11 core, C++17 SDK boundary, single owner; profile/compile graph/IRAM/RAM reviewed. Реальна електрична придатність відкрита |
| IV — Reproducible Builds and Evidence | Після останньої firmware зміни fresh host й9 clean pinned builds; source/artifact SHA256, commands/exits/failed attempts збережені. Runtime/physical не заявлено |
| V — Bounded Iterations and Authorized Actions | Тільки явно запитаний Implement04; workflow directory явно заданий. Гілка/HEAD не змінені, commit/push/flash/monitor/load/наступні фази не виконувались; hooks файл відсутній до/після |
| VI — Honest Decisions and Uncertainty | D59/D60/D61/D62 виконано без повторного вибору параметрів; Q02/Q03/Q06 та owners/next gates явні, Q05→05, motor/power/thermal→06 |

## Невиконане та обсяг завершення

**46/53 задач виконано**: T001–T043 і T051–T053; проміжний звіт дозволений задачами при hardware blocked. **T044–T050 unchecked**, власник — оператор стенду за окремим явним апаратним завданням. Спочатку Q02/Q03/method readiness, потім Q06 actual driver/jumper, далі startup→waveform→fault→stress→accuracy. Детальні owner/reason/next evidence — [hardware.md](hardware.md).

Існуючі user changes у prompts05–09 та speckit-iteration/README.md збережено за початковими hashes; `.specify`, skills/constitution і prerequisite artifacts не змінені. Межі hashes та preservation check зафіксовано у software-evidence.json. Загальна ітерація04 очікує фізичного прийняття; цей результат завершує доступний software/build обсяг запитаного Implement.
