# План перевірки 02

Це інструкція для майбутньої реалізації, не звіт про виконання. Не запускати Tasks/Implement або апаратні дії лише на її підставі. Перед виконанням перевірити наявність реалізованих control source/test та актуальність `tools/environment/environment.lock.json`; якщо pinned шляхи відсутні, записати blocked і відновити середовище окремо.

## Host без MCU

Після реалізації, з кореня репозиторію, із новим каталогом:

```sh
python3 scripts/build.py host --build-dir build/host-02-new
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-02-new --output-on-failure
```

Очікується окрема suite для ядра з перевірками всіх шести точок, 899/900/2100/2101 і крайніх цілих значень, nominal endpoints, всіх меж округлення ±1 мкс, regression invalid-to-high, утримання, capture fault, valid під latch, clear та початкового/disarmed 0%. Fake clock перевіряє last-valid, повторні й зворотні timestamps, зокрема latch capture fault навіть при регресії часу; public-header probe і незалежне компонування core без SDK/платформних header. Нові CMake targets мають бути додані реалізацією до запуску. Фактичні команди, середовище та результати записати у `verification/host.md`.

## Target build без підключення

```sh
python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-02-new
python3 scripts/verify/config.py --build-dir build/c3-02-new
```

Очікується окремий native C3 build із новим source у component graph, без network/radio і без operational arming; результати та hashes — `verification/build.md`. Якщо environment або джерела відсутні, це blocked, не pass. Build не означає flash або фізичну перевірку.

## Фізичний запис

У `verification/hardware.md` записати not_run/pending та залежності, доки окремо не погоджено operator steps, прилад і числові критерії. Для 01 відкрито H02 живлення/bias та H03 waveform; 03 має підтвердити RC levels/timing, 04 — PWM і перевірити постійну перемичку DIR–GND без насоса, 05 — failsafe і 06 — power path та насосний стенд. Під час 02 плата не прошивається і навантаженням не керують. За D38 02 може завершитися за власними критеріями після перевірки потрібної програмної основи 01, не очікуючи повного фізичного прийняття 01; записи not_run залишаються відкритими. Host і build не доводять фізичного cutoff, а MD20A LOW означає brake.
