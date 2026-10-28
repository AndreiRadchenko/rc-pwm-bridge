# Огляд меж ядра та конфігурації

2026-10-28, T021. Переглянуто `firmware/components/bridge_core/include/bridge/control.hpp`, `control.cpp`, `firmware/main/app_main.cpp`, CMake compile commands і результат `scripts/verify/config.py`.

- Control core містить лише `stdint.h`/власний header; host `clang++ -MM` виявляє лише control.cpp і control.hpp як проєктні залежності. Реалізація не використовує платформні API, I/O, heap, endpoint learning чи timeout.
- `work_permitted` початково false й не змінюється; `effective_duty_percent` початково 0 й не змінюється; `clear_fault` лише повертає disarmed. `app_main.cpp` зберігає 01 startup supervisor без control arming.
- Автоматичний config/graph/map/source review — exit 0, без радіомереж у перевіреному graph. Він не замінює фізичних startup/watchdog/bootloader перевірок; поточна control логіка не викликається target app.
- Q05 не визначено, physical H02/H03 залишаються відкритими. LOW MD20A — brake, незалежний power cutoff окремий.
