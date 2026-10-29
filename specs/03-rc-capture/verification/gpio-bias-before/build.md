# Target builds 03

2026-10-29. **Три native ESP32-C3 builds pass**, плата не потрібна й не використовувалась. SDK ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, GCC14.2.0 esp-14.2.0_20260121, CPython3.11.7, CMake3.30.2, Ninja1.12.1. Нових зовнішніх залежностей немає. Pins01 збережено; environment checker пройшов.

## Команди та результати

```sh
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-final
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-stress --rc03-bench diagnostics
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-trace --rc03-bench accuracy
python3 scripts/verify/config.py --build-dir build/c3-03-final
python3 scripts/verify/config.py --build-dir build/c3-03-stress
python3 scripts/verify/config.py --build-dir build/c3-03-trace
```

Усі directories спочатку створені новими через build.py. Після review-виправлень виконано інкрементальну збірку кожного через pinned `cmake --build <directory>` із тими самими IDF_PATH/IDF_TOOLS_PATH/IDF_PYTHON_ENV_PATH/PATH та ESP_ROM_ELF_DIR із manifest. Це фінальний native build результат, не заява про clean rebuild фінальної ревізії. Початкові full logs — build-final/stress/trace.txt; фінальні compile/link logs — rebuild-final/stress/trace.txt. Невдалі проміжні compile attempts виправлені: strict-C++ incompatibility HAL typeof → пряме C3 pending register read; явне FreeRTOS.h перед queue.h; правильна ISR yield signature. Вони не є поточними blockers. Фінальні rebuild logs без compile warnings/errors.

| Варіант | Stress / trace | App bin B | App .dram0.bss B | Capture object B |
|---|---|---:|---:|---:|
| c3-03-final | off/off |172656|25336|16336|
| c3-03-stress | on/off |172656|25336|16336|
| c3-03-trace | on/on |173360|31544|22544|

У всіх: .iram0.text56438B, .dram0.data5096B. Різниця trace6208B, record48B/header64B, нижче6400B bound. Diagnostic object4752B. Capture object містить static8192B worker stack, queue storage/control, scratch192B, policy, metadata; diagnostics містить4096B stack. Main3584B, esp_timer3584B, ISR1536B — effective SDK defaults, не виміряні high-water marks. Static sections не є вимірюванням залишку heap/runtime stack.

Bin SHA-256:

- normal: `b4f90f0a97962270b08d07a127ad07a4bcfc58a97831e977dfb012a40d63e428`
- stress: `829341b996a5c1f9f6a794c320df0d4bd393d2b558ca94390d4a09021ab0f652`
- trace: `8b4990bccb2847ad7d9f54bad379b0f18211a071f3e10832584241dd5bc55df2`

Повний source/config/graph/compile/map/ELF/bin/bootloader/partition hashes — [software-evidence.json](software-evidence.json), секції — sections-*.txt. Build artifacts лишаються у власних ignored directories; hashes не замінюють самі binaries перед flash. Старі01/02 artifacts не змінювались.

## Межі

Config/graph review — [config-review.md](config-review.md). Збірка не вимірює ISR timestamp accuracy, rearm/processing/deadline, zero loss600с, реальний USB backpressure, GPIO0 boot/reset чи H02/H03/Q03. Ці перевірки **not_run** у [hardware.md](hardware.md). IDF виводить довідкові flash commands у full build log; вони не виконувались.
