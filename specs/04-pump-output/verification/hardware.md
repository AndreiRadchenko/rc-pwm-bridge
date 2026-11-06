# Фізичні перевірки04

2026-10-30. **not_run / blocked.** Цей Implement не відкривав пристроїв, не прошивав/не скидав MCU й не підключав драйвер або насос. Raw analyzer/analog records та фізичні report відсутні. Усі операторські T044–T050 лишаються unchecked. [Порядок і допуски](../quickstart.md#послідовність-фізичного-прийняття-без-насоса), [підготовка](hardware-procedure.md).

| Gate / задача | Стан | Власник і наступний доказ |
|---|---|---|
| Q02 / H02–H03 / T044 | blocked | Оператор стенду: живлення3,3В, PWM bias, startup до firmware, USB/external power режим на фактичній платі |
| Q03 / T044 | blocked | Оператор: HJ conditioning/рівні/зовнішній bias та залежне фізичне прийняття03; host/config не доводять електричну придатність |
| Q06 / T045 | blocked | Оператор: фактична ревізія MD20A, interface/земля/power/bias/кнопки, постійна DIR–GND перемичка без насоса |
| M04 / T044 | not_run | Оператор: модель/калібрування приладу, sampling/storage/trigger/U та method provenance до запуску |
| Startup/reset/bootloader/upload / T046 | not_run | Оператор після gates: normal/bench, disconnected-driver і powered-driver, щонайменше5 повторів case, повний startup/LOW/voltage |
| Шість точок/endpoints/1000Гц / T047 | not_run | Оператор: обидві точки вимірювання, waveform raw + аналогові рівні, M04 per-cycle/transition criteria |
| Fault→LOW / T048 | not_run | Оператор: supported stuck cases, GPIO3/0≤112мс та≥10с terminal guard; MCU1/2мс API brackets окремо |
| Stress / T049 | not_run | Оператор: active/slow/absent reader, 601 MCU-секунда, lower-bound≥600 фізичних секунд, source matching, output traces, runtime stack high-water |
| Capture accuracy / T050 | not_run | Оператор:128-candidate windows для trace-steps0..5, D53–D55 per-pulse comparison, LOW-before-stop, reader conditions |

Невідомі gate або ревізія блокують залежне підключення/живлення. Проміжні disconnected-driver спостереження не підміняють powered-driver прийняття. Невідомий receiver не додається як gate для HJ-only тестів. Сам факт успішної target build не підтверджує запуск програми на платі.

Q05 operational arming/watchdog належить05; повний brownout/watchdog, напрямок реального насоса, силовий тракт/current/thermal/flow та незалежне emergency cutoff — відповідним05/06 gates. Тут їх не реалізовано й не прийнято. PWM LOW MD20A означає brake, не coast, ізоляцію живлення чи доведений нуль витрати.
