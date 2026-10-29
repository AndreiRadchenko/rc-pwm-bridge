# Target builds 03

2026-10-29. **Три native ESP32-C3 builds pass**, плата не потрібна й не використовувалась. SDK ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, GCC14.2.0 esp-14.2.0_20260121, CPython3.11.7, CMake3.30.2, Ninja1.12.1. Нових зовнішніх залежностей немає. Pins01 збережено; environment checker пройшов.

## Команди та результати

```sh
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-bias-normal
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-bias-stress --rc03-bench diagnostics
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-bias-trace --rc03-bench accuracy
python3 scripts/verify/config.py --build-dir build/c3-03-bias-normal
python3 scripts/verify/config.py --build-dir build/c3-03-bias-stress
python3 scripts/verify/config.py --build-dir build/c3-03-bias-trace
```

За D56 усі три directories створено заново через build.py; три clean target builds завершились exit0. Попередні build/c3-03-final/stress/trace збережено без перезапису. Їхні records/hashes перед зміною збережені у gpio-bias-before/. Поточні full logs — build-bias-normal/stress/trace.txt; config-*.txt, sections-*.txt і software-evidence.json відповідають новим bias builds. У логах є довідкові flash commands; прошивання не виконувалось.

У compiled ELF перевірено GPIO3/режим GPIO_FLOATING (обидва аргументи3), порядок new channel→pull mode→callbacks→enable і error branch до rmt_del_channel. Excerpts — gpio-bias-acquire-normal/stress/trace.txt. SDK GPIO_FLOATING вимикає обидві внутрішні підтяжки. Ці перевірки не є вимірюванням фізичних рівнів.

| Варіант | Stress / trace | App bin B | App .dram0.bss B | Capture object B |
|---|---|---:|---:|---:|
| c3-03-bias-normal | off/off |173200|25336|16336|
| c3-03-bias-stress | on/off |173200|25336|16336|
| c3-03-bias-trace | on/on |173904|31544|22544|

У всіх: .iram0.text56438B, .dram0.data5096B. Різниця trace6208B, record48B/header64B, нижче6400B bound. Diagnostic object4752B. Capture object містить static8192B worker stack, queue storage/control, scratch192B, policy, metadata; diagnostics містить4096B stack. Main3584B, esp_timer3584B, ISR1536B — effective SDK defaults, не виміряні high-water marks. Static sections не є вимірюванням залишку heap/runtime stack.

Bin SHA-256:

- normal: `aa370d2981c638080201c9acf3bf93a972f0366c64a939f8f54a844b4f2851b7`
- stress: `51e99f196f12e31a878b0a15e133a4d41b5ff00718e45fa1df551b8fd3d5d538`
- trace: `1b556e0f92673bc2a8a81cea6eaf97115bc5f6d04d00a344d63a10ad7099ae60`

Повний source/config/graph/compile/map/ELF/bin/bootloader/partition hashes — [software-evidence.json](software-evidence.json), секції — sections-*.txt. Build artifacts лишаються у власних ignored directories; hashes не замінюють самі binaries перед flash. Старі01/02 artifacts не змінювались.

## Межі

Config/graph review — [config-review.md](config-review.md). Збірка не вимірює ISR timestamp accuracy, rearm/processing/deadline, zero loss600с, реальний USB backpressure, GPIO0 boot/reset чи H02/H03/Q03. Ці перевірки **not_run** у [hardware.md](hardware.md). IDF виводить довідкові flash commands у full build log; вони не виконувались.
