# tests

`tests/host` — незалежні від MCU тести C++11 supervisor і C++17 політики; `tests/tooling` — перевірки профілю, CLI та config evidence. Чистий host build використовує pinned macOS arm64 tools через `python3 scripts/build.py host --build-dir <новий-каталог>`. CTest запускається відповідним pinned `ctest`, negative probe — окремо і має завершитися ненульовим кодом.

Фактичні результати — [host.md](../specs/01-foundation/verification/host.md), [build.md](../specs/01-foundation/verification/build.md). Host-тест не доводить фізичний стан GPIO.


03 додає10 CTest suites (разом18): capture header/validation/association/core/fault/time/adapter/trace і diagnostics policy/transport. Усі назви, конкретні boundaries та обмеження fake tests — [host03](../specs/03-rc-capture/verification/host.md). Tooling запускається `python3 -m unittest discover -s tests/tooling -v` (17 tests); він не відкриває device. Offline comparator `scripts/verify/rc_capture_trace.py` використовує Python stdlib, Decimal, frozen128-record archive та normalized analyzer CSV; приклади/протокол — [quickstart03](../specs/03-rc-capture/quickstart.md). Synthetic E4.6/U0.8 pass перевіряє правило D54 без E+U guard band, не точність фізичного пристрою.


04 додає `pump_output_policy_tests`, `pump_output_header_probe`, `pump_output_adapter_tests`, `pump_output_evidence_tests`: разом22 CTest suites і28 tooling tests. Adapter tests виконують production C++17 adapter через fake SDK; policy/header залишаються strict C++11. Evidence tests перевіряють frozen archives, canonical digest, RAM/payload bounds і production writer cursor: partial write не зсуває кадр, повний цикл проходить H..R..E й чергується з win/trace. Fake tests не доводять реальні GPIO/timing/stack high-water.

`scripts/verify/pump_output_trace.py` працює тільки з файлами, без пристрою. Аргументи `--archive --edges --metadata --report`; exit0=pass запитаних метрик,1=fail,2=blocked. Схема metadata, CLI/build variants, окремі операторські gates та спосіб збереження повних out/win cycles — [quickstart04](../specs/04-pump-output/quickstart.md). Фактичні свіжі результати й artifact hashes — [host04](../specs/04-pump-output/verification/host.md), [build04](../specs/04-pump-output/verification/build.md), [acceptance04](../specs/04-pump-output/verification/acceptance.md).
