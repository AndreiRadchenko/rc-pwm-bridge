# Програмна перевірка 02

2026-10-28. Область: переносиме ядро за R03/R04/R05/R12; D38 дозволяє завершення 02 без повного фізичного прийняття 01. Реальні host/build результати і непроведена фізична частина записані окремо.

| Вимога / критерій | Задачі та докази | Результат |
|---|---|---|
| R03 / FR-001,006 / US3, SC-003/004 | T003–T006,T014–T018,T021; `host.md`, `config-review.md` | Host 8/8, core-only probe, disarmed/fault/clear 0%, permission false. |
| R04 / FR-002,004 / US1, SC-001 | T007–T009,T018; `host.md` | Шість точок, endpoints та 100 меж округлення з сусідами; host pass. |
| R05 / FR-003,005,006 / US2–3, SC-002/003 | T010–T018; `host.md` | Межі й invalid-to-high, відновлення valid, capture fault, час і clear — host pass. |
| R12 / FR-007, SC-004 | T001,T002,T018–T023; `prerequisites.md`, `environment.md`, `host.md`, `build.md`, `hardware.md` | Host і target compile/build pass; фізичні докази not_run в окремому записі. |

Програмні критерії 02 виконано за наведеними доказами; ControlCore скомпільовано для target, але app його ще не викликає — інтеграція capture/output та operational safety належать наступним етапам. Це не фізичне прийняття. 01 H02/H03 та SC-004/SC-006 відкриті, Q03–Q06 мають власні gates; MD20A/насос не підключалися, команда насосу не дозволена. Збірка не доводить фізичний cutoff.
