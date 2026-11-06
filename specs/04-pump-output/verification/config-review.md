# Config, ELF, ISR та RAM review04

2026-10-30. Фінальні nine `build/c3-04-reviewed-*`: config verifier **9/9 exit0**, [команди](config-results-reviewed.json). Actual sdkconfig/project graph/compile_commands/map перевірено; manifest source hashes для кожного образу збігаються з фінальними firmware sources. [ELF audit](audit_elf.py) через pinned objdump `-t -d -C`: [результати](iram-results.json), деталізація iram-audit-*.json. Це статичні software докази, не runtime/physical acceptance.

## Конфігурація та ownership

C3/DIO40МГц/4MB/single factory1MB, native USB Serial/JTAG, logs0/tick100Hz. Core actual objects закінчуються `-std=c++11 -fno-rtti -fno-exceptions`, platform/main strict C++17. Graph містить native gpio/rmt/ledc; radio/network archives/init symbols відсутні. PM/tickless sleep off. Нові залежності не змінюють pinned ESP-IDF5.5.6. Capture GPIO3/RMT APB1МГц/48symbols/5ms/filter0/non-DMA/non-partial/IRQ3, GPIO_FLOATING D56 та thresholds/core mapping02–03 збережено.

Output GPIO0 тільки через EspPumpOutput/EspPumpSdk; MCU DIR немає. Погоджений профіль перевіряється wrapper і startup; DIR–GND фізично ще не оглянуто. LEDC LOW_SPEED timer0/channel0/APB80МГц/13bit/1000Гц/hpoint0. Проміжний count=(percent×8192+50)/100; 0/100 обходять PWM count і використовують stop idle0/idle1. У linked ELF відсутні `ledc_fade_func_install` і `ledc_set_duty_and_update`: згадки цих SDK функцій у discarded sections/cross-reference map не є включенням. Regression перевіряє обидва випадки. `ledc_set_duty_with_hpoint`/`ledc_update_duty` не встановлюють fade service; pinned `_ledc_fade_hw_acquire` без fade record повертається без очікування fade semaphore. Чужих LEDC clients у композиції немає.

Startup owner спочатку задає GPIO LOW/direction, потім перевіряє init/diagnostics/profile; до capture handoff він є єдиним output writer. Після успішного start app_main більше не poll/write supervisor; capture/core owner priority10 обслуговує policy й SDK між bounded блоками. SDK не викликається з application metadata lock або ISR. Fault checks перед/між/після nonzero calls, sticky first fault time, stop0→GPIO level0→directionOUTPUT fallback. За pinned gpio.c gpio_output_enable повертає matrix до simple GPIO перед enable; після fallback автоматичного повернення до LEDC немає. Init/API/deadline failures terminal; неповний LOW path=LowUnconfirmed. Cleanup спершу LOW, потім pause/deconfigure; жоден retry не відновлює bench permission. Core work_permitted=false/effective0, calculated duty окремо.

Bench default off. Normal не має evidence/window/trace allocation і не конфігурує LEDC output sequence. Waveform/stress/accuracy — окремі compile selections; accuracy sentinel−1 відхиляється, wrapper потребує явний0..5. Reset reason informational: bench кожного app boot починає лише нову сесію від initialLOW5с; власного restart немає. Ненульова capture/USB команда не може обрати bench. Accuracy LOW перед stop/export; stop/API failure не видається за capture_active=false успіх. Stress capture/max diagnostics тривають після output Complete605с до closure606с+guard, а після freeze capture продовжується.

## ISR closure та internal data

У кожному фінальному ELF перевірено **43 functions** у direct closure, всі `.iram0.text` або ROM `*ABS*`. Це42 попередні functions03 плюс явно доданий root `systimer_ticks_to_us`, який раніше розв’язувався вручну як indirect HAL target. Audit також розпізнає auipc/jalr із resolved ROM target у objdump. Unknown indirect paths лишаються: GPIO service→зареєстрований gpio_edge, RMT done/threshold→зареєстрований rx_done (partial вимкнено), esp_timer_get_time→systimer_ticks_to_us. Їх розв’язано за pinned SDK/registration/config. Flash output/core/formatters/USB у ISR closure відсутні.

Новий producer counter лише коротко increment/decrement під тим самим ISR metadata lock навколо queue publication; без SDK output, heap або форматування. Closure guard враховує producers, queue drain, pending edge/fault і належний active receive. Стабільність guard/latency під реальними IRQ ще належить T049.

RcCapture, static queues/scratch/lock/callback context, LocalDiagnostics, output/evidence/window — `.dram0.bss`; gpio_context `.dram0.data`, systimer_hal `.dram0.bss`. Pinned cache-safe RMT/GPIO allocation лишається INTERNAL RAM. Callback batch на internal ISR stack; GPIO frame64B, RMT480B, arm32B. Новий collector не розміщено на owner stack.

## Bounds

| Actual C3 static allocation | Normal | Waveform | Stress | Accuracy0–5 |
|---|---:|---:|---:|---:|
| OutputEvidence |0|2616B|2616B|2616B|
| CaptureWindowEvidence |0|0|240B|0|
| RcCapture (включно з task stack) |16432B|16432B|16432B|22640B|
| LocalDiagnostics (включно з task stack) |4808B|4808B|4808B|4808B|
| EspPumpOutput |88B|88B|88B|88B|

OutputEvidence2616≤2688B (64×native40B +header56B), stress2616+240=2856≤3200B, accuracy додатковий trace6208 +2616=8824≤9088B; normal history не створює. Compile-time asserts також обмежують record40/header128/window512/snapshot384. Canonical record36B і window144B не залежать від padding/endianness. Snapshot coherent; rc04 max383B/padded384B, TX512/RX128, один in-flight/≥100мс/zero-wait/latest-only. Export тільки frozen pointers; writer-only cursor retries partial кадр і чергує повні цикли. No per-pulse USB/history/steady-state application heap.

Actual explicit task frames: capture step1072B, output_service448B, diagnostics step1344B. IRQ stack1536B, capture task8192B, diagnostics4096B збережені. Ці frames не сумуються в доказ runtime high-water й не охоплюють усі SDK/fatal nesting: **runtime high-water not_run/T049**. Статичні bounds не доводять відсутність переповнення при всіх фізичних IRQ.

SDK watchdog/bootloader/panic settings01 не змінено. Повний brownout/watchdog належить05/06, фізичний bias/startup Q02/H02/H03 та Q03/Q06 відкриті. API-success означає тільки успіх виклику; LOW MD20A — brake, без power isolation/coast/flow claims.
