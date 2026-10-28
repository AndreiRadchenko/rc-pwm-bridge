# Host-перевірка 01

2026-10-28; виконавець Codex; R02/R08/R12, FR-003/007/008/010/011/014/015, SC-003. Середовище macOS arm64, фактичні версії та hashes — `tools/environment/environment.lock.json`. Плата/SDK не використовувались.

| Команда | Очікування | Фактично |
|---|---|---|
| `python3 scripts/build.py host --build-dir build/host-01-evidence` | clean host build, exit 0 | exit 0, 10 build steps |
| `/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-01-evidence --output-on-failure` | startup_tests і core_header_probe pass | exit 0, 2/2 passed |
| `build/host-01-evidence/negative_probe` | навмисна помилка, nonzero | exit 1, `line 3: s.snapshot().work_permitted` |

Каталог `build/host-01-clean` спочатку використано для розробки й повторної збірки; для остаточного clean запуску створено новий `host-01-evidence`, без видалення зайнятого каталогу. Перевірено запуск, 5 невдач, успіх третьої, completion-relative deadline, late tick, повтор, reset, regression, overflow і protocol fault; fake ports перевіряють порядок, помилки clock/profile/output/setup та one-attempt/drop без replay. Це host evidence, не вимірювання GPIO.

`compile_commands.json` для core: `-std=c++11 -fno-rtti -fno-exceptions`, без GNU extensions; CMake CXX_EXTENSIONS NO. Команда `clang++ -std=c++11 -MM -Ifirmware/components/bridge_core/include firmware/components/bridge_core/startup.cpp` показала лише startup.cpp і startup.hpp як project dependencies; системні стандартні headers опущені -MM. Пошук core source на new/delete/malloc/calloc/realloc/free і memory/vector/string не знайшов збігів. Target-core flags, SDK graph і фізична поведінка залишаються неперевіреними.

## SHA-256 джерел цього запуску

```text
33f863169328e5e792716ebb18b55e321279adfde5e745a85ddf95f06158fc12  firmware/components/bridge_core/include/bridge/startup.hpp
f51eb6835b882d7a98b8c2cc7ea58ef669c78592ced0990477dc63b2cce89ebc  firmware/components/bridge_core/startup.cpp
0cac1528c7319c9cf506bbdfb43632fc3d435bca9b2542a6bc5f48fbc5eea2ab  firmware/components/platform_esp32/include/platform/startup_port.hpp
019743055e92c98f9ea5ce6f02f6cbcea865c3122ffe2c07b45e151aa8a60bfc  firmware/components/platform_esp32/startup_policy.cpp
bb4db8e1705e1f0aaf77828fb52a37f0bbc975856025c8c58472e0eb14688a86  tests/host/CMakeLists.txt
ca60ade0468236e21ec2fdd703fbc9d78d75edab0f05f452d18b747dd95901c5  tests/host/startup_tests.cpp
5e85f948381076dd65cfd5a229fc82a567bfa35df76ac59fc46a52c27a6e7699  tests/host/test_support.hpp
38c6d5c34ca8f8f0dba38c250f6de3b0129f0d7afed732ffd9f390b83c2a72f6  tests/host/core_header_probe.cpp
fe94f60a347a5e3c58d3f5959eef2bc5c45f82da13e3d4900c6e0302ce8df209  tests/host/negative_probe.cpp
d3b08a47734298340b8d10f5a942d70133c65257451afb71bdedcbb31a69ba14  scripts/build.py
5ff79950d42727bb80e933c29b014a386b1e5882002db59d1502e2ea2142fb77  tools/environment/environment.lock.json
```

## Повтор після оновлення environment manifest

`python3 scripts/build.py host --build-dir build/host-01-environment-updated` — exit 0; CTest — exit 0, 2/2; `build/host-01-environment-updated/negative_probe` — exit 1 із очікуваним failed check. Оновлений SHA-256 manifest: `ea32a067878e9a1f7eb7c3aceb3b9644ecce70f5412bce6cddd3bc6a010e747c`. Решта перевірених host sources не змінювалася. Нові SDK leaf adapters не входять у цей host build.

## Повтор після реалізації target tooling

`python3 scripts/build.py host --build-dir build/host-01-final` — exit 0; CTest — exit 0, 2/2; `negative_probe` — exit 1, очікувана помилка. Після попереднього результату змінилися лише wrapper та target environment manifest; host core і host policy не змінювалися. Актуальні hashes:

```text
b94e8515f306a817a9fc72a20ebcc530eb944b15956dc302ab32f5a27c58ef1f  scripts/build.py
1a6966611cd586c9a9497c4c3cf3e393e89bffd29ab1d9fb185b2f347550b0cc  tools/environment/environment.lock.json
f51eb6835b882d7a98b8c2cc7ea58ef669c78592ced0990477dc63b2cce89ebc  firmware/components/bridge_core/startup.cpp
019743055e92c98f9ea5ce6f02f6cbcea865c3122ffe2c07b45e151aa8a60bfc  firmware/components/platform_esp32/startup_policy.cpp
ca60ade0468236e21ec2fdd703fbc9d78d75edab0f05f452d18b747dd95901c5  tests/host/startup_tests.cpp
```
