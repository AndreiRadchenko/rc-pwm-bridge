# tests

`tests/host` — незалежні від MCU тести C++11 supervisor і C++17 політики; `tests/tooling` — перевірки профілю, CLI та config evidence. Чистий host build використовує pinned macOS arm64 tools через `python3 scripts/build.py host --build-dir <новий-каталог>`. CTest запускається відповідним pinned `ctest`, negative probe — окремо і має завершитися ненульовим кодом.

Фактичні результати — [host.md](../specs/01-foundation/verification/host.md), [build.md](../specs/01-foundation/verification/build.md). Host-тест не доводить фізичний стан GPIO.


03 додає10 CTest suites (разом18): capture header/validation/association/core/fault/time/adapter/trace і diagnostics policy/transport. Усі назви, конкретні boundaries та обмеження fake tests — [host03](../specs/03-rc-capture/verification/host.md). Tooling запускається `python3 -m unittest discover -s tests/tooling -v` (17 tests); він не відкриває device. Offline comparator `scripts/verify/rc_capture_trace.py` використовує Python stdlib, Decimal, frozen128-record archive та normalized analyzer CSV; приклади/протокол — [quickstart03](../specs/03-rc-capture/quickstart.md). Synthetic E4.6/U0.8 pass перевіряє правило D54 без E+U guard band, не точність фізичного пристрою.
