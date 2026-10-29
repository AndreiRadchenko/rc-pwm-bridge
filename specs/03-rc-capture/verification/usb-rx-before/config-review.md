# Config, graph, ISR/data review 03

2026-10-29. Config verifier exit0 для normal/stress/trace, логи config-*.txt. Негативні fixtures перевіряють actual generated configs/compile graph, missing capture object/IRAM flag, trace object/config mismatch і неправильний C++ standard; не лише наявність рядків source. Tooling17/17 pass.

## Effective configuration та compatibility

C3/DIO40МГц/4MB/single factory1MB; native USB Serial/JTAG/secondary none; application/bootloader logs0; tick100Hz. RMT_RX_ISR_CACHE_SAFE, RMT_RECV_FUNC_IN_IRAM, GPIO_CTRL_FUNC_IN_IRAM=y. SDK виводить також RMT_OBJ_CACHE_SAFE/RMT_RX_ISR_HANDLER_IN_IRAM=y; PM і tickless idle off. У graph/map відсутні esp_wifi/bt/esp_netif/lwip та їх init symbols. Native esp_driver_rmt/esp_driver_gpio присутні. Non-DMA/non-partial, APB1MHz/filter0/max5ms/48symbols задані адаптером; IRQ3 RMT/GPIO. Вже встановлений чужий GPIO service не переуживається: flags/priority невідомі, startup fault/retry без видалення чужого service.

Усі actual bridge_core objects мають останній -std=c++11, no RTTI/exceptions; platform/app strict C++17. Capture sources наявні в усіх graph; rc_capture_trace.cpp лише accuracy graph. Normal та diagnostics-stress не містять trace collector object. USB archive methods без trace flag не викликаються і не створюють RAM history. API/поведінка ControlCore02 не змінені. DiagnosticSnapshot/текст розширено, old legacy DiagnosticConsumer лишився для01 context; composition03 використовує RcDiagnosticPolicy D43.

## ISR call graph та internal data

Перевірено pinned riscv32-esp-elf-objdump -t/-d/-h по кожному фінальному ELF; адреси/секції, direct call/jump closure — iram-audit-final/stress/trace.json (42 functions для кожного). Roots: GPIO service, RMT default ISR і обидва application callbacks. Кожна знайдена direct функція в .iram0.text або ROM *ABS*, flash .text callees немає. Це static review, не runtime cache-off/latency test.

Application GPIO/RMT callbacks, arm_locked, pending_edge — .iram0.text. Вони викликають esp_timer_get_time, gpio_get_level/intr_disable, vPortEnter/ExitCritical, queue send/notify/yield FromISR, rmt_receive. Транзитивно перевірено queue/list helpers, atomic_compare_exchange4, systimer counter getter, ROM udivdi3/memset, IRAM memcpy та SDK assert/panic paths. Немає heap, core calls, форматування, USB або task waits у application ISR. Callback копіює максимум48symbols у stack batch до повторного rmt_receive; черги64/8 мають static storage, enqueue failures накопичуються поза чергою. GPIO overflow маскує ingress. Метадані arm/close/epoch/seq/recovery та pending losses захищено одним lock; RMT/GPIO IRQ однакового priority3 на single-coreC3.

Indirect calls розв’язано за pinned SDK/source/config:

- gpio_isr_loop → зареєстрований gpio_edge, єдиний handler service цієї композиції; чужий service відхиляється.
- rmt_isr_handle_rx_done/threshold → зареєстрований rx_done; partial RX вимкнено, альтернативні callbacks не задані. SDK перемикає FSM до ENABLE перед final callback, дозволяючи callback rearm; scratch уже скопійовано.
- esp_timer_get_time → systimer_hal.ticks_to_us = systimer_ticks_to_us, символ у .iram0.text. HAL context у .dram0.bss. Absolute clock synchronization не додається.

RcCapture normal16336B/trace22544B і LocalDiagnostics4752B розміщені .dram0.bss. Scratch/queues/config/lock/callback context — members цих internal objects; callback batch — ISR stack internalRAM. gpio_context — .dram0.data, systimer_hal — .dram0.bss. IDF RMT_MEM_ALLOC_CAPS за cache-safe config = MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT; GPIO ISR service з IRAM flag виділяє handler table MALLOC_CAP_INTERNAL. Функціональні handlers не читають flash strings/таблиці/vtable. SDK assertion/panic є окремим fatal path зі штатною cache-aware обробкою, його фізичну поведінку не перевірено.

Перегляд bounds: GPIO callback frame64B, RMT callback frame480B, arm frame32B за disassembly; direct calls bounded у application, RMT48 loop і queue copy fixed size. IRQ1536B та task stacks8192/4096B — явні/SDK budgets. Статичний frame review не доводить high-water при всіх IRQ/SDK paths; runtime stack/timing під stress залишається операторською перевіркою. Setup/recovery SDK allocations допустимі; application steady-state heap allocations відсутні. Partial init cleanup проходить ownership bitmap у reverse order; init/recovery retry≥100ms, без нового app reset policy.

## Startup, watchdog та safety

app_main чекає ReadyDisabled supervisor01 перед capture.start; apply_low GPIO0 перед setup, DIRGPIO немає. Capture/core owner один priority10; timer task лише notify1000us, diagnostics priority1. Вихідний PWM/arming/clear fault не створювались; core effective0/permissionfalse завжди. Захоплення продовжується без reader. За startup API failure ресурси cleanup/retry, fault/latch не скидаються.

Bootloader WDT9000ms; interrupt WDT300ms; task WDT initialized5s/idleCPU0, task WDT panic off; panic print/reboot. Це штатні SDK settings01, не прийнятий operational watchdog05. Не додано esp_restart/application retries зі скиданням MCU. ROM/bootloader/panic/upload/reset можуть впливати на GPIO до/поза app — H02/H03 та фізичне прийняття лишаються відкритими. GPIO0LOW на MD20A — brake, не power isolation чи доведений нуль витрати.

Поточний verify/config очікує graph03; старі збережені01/02 builds не перевіряються ним повторно як нові capture builds. Їхні історичні результати й artifacts збережені, current source потребує нової збірки03.

Final review: queue send/copy винесено поза metadata lock; лише узгодження seq/epoch/line/recovery/arm і коротке notify під lock. Snapshot.phase і незалежні ValidationResult flags доступні без IDF; clock regression не залишає core Disarmed без latch — fault упорядковується останнім достовірним часом, без зміни last-valid або заяви виміряного deadline.

## D56: зовнішній bias GPIO3

Три нові build/c3-03-bias-normal/stress/trace пройшли effective config/graph checks. gpio_set_pull_mode(input,GPIO_FLOATING) стоїть після успішного rmt_new_rx_channel, перед register callbacks/rmt_enable. Відмова виклику потрапляє у спільний cleanup: rmt_del_channel/channel=nullptr/return false, далі штатний fault/retry lifecycle; re-init повторно задає bias. Повторний rmt_enable/receive не створює канал і не вмикає pull-up знову за pinned SDK. GPIO routing/input-enable та GPIO0 не змінюються.

У actual ELF кожного варіанта перевірено li a1,3;mv a0,a1;call gpio_set_pull_mode;conditional branch до cleanup. У pinned driver GPIO_FLOATING=3 виконує gpio_pulldown_dis і gpio_pullup_dis; помилки перевіряє adapter. Повні acquire excerpts — gpio-bias-acquire-*.txt. Цей setup call task-only, не ISR: direct closure ISR лишається42 IRAM/ROM functions, static data/stacks без збільшення. Старі records у gpio-bias-before/. GPIO registers/напруги на фізичній платі не читались; Q03/physical gates залишаються відкритими.
