# tests

`tests/host` — незалежні від MCU тести C++11 supervisor і C++17 політики; `tests/tooling` — перевірки профілю, CLI та config evidence. Чистий host build використовує pinned macOS arm64 tools через `python3 scripts/build.py host --build-dir <новий-каталог>`. CTest запускається відповідним pinned `ctest`, negative probe — окремо і має завершитися ненульовим кодом.

Фактичні результати — [host.md](../specs/01-foundation/verification/host.md), [build.md](../specs/01-foundation/verification/build.md). Host-тест не доводить фізичний стан GPIO.
