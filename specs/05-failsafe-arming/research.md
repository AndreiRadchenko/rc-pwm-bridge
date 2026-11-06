# Дослідження 05: свіжість, відновлення та нагляд

Дата:2026-11-06. Обсяг — Phase0 запитаного Plan; документація, читання коду й первинних джерел. Три research agents працювали read-only за speckit-plan: capture/core migration, output recovery, watchdog. Firmware, SDK, гілка, обладнання не змінювалися; нових host/build/physical результатів немає. Початкові шість gates записані в plan.md до dispatch. На момент Phase0 G05 блокував implementation; окреме пізніше Constitution-завдання D74 ухвалилоv2.0.0 та закрилоG05. Нижче збережені джерела й факти research до поправки, без повторного вибору технічних параметрів.

## RCH-01 — Вхідні рішення й залежності

**Рішення**: застосувати D63–D68 без переобрання. Test default/Fly explicit, no arming/zero-handshake/manual input-loss clear. Загальна Test межа last-valid-end→physical LOW≤1 с; age≥T. Технічні вибори нижче зроблені саме в делегованому Plan, не видані за попередньо погоджені числа або вимірювання.

**Підстава**: [spec05](spec.md), [поточна основа](../../speckit-iteration/current-baseline.md), поточні docs/decisions,requirements,architecture,roadmap,open-questions, конституція v1.0.0, AGENTS.md, hardware/README. Прочитані prerequisite spec/contracts01–04 і spec/plan04; точні посилання та evidence boundaries збережені в spec05. Acceptance01 має software evidence й відкриті H02/H03,02 виконав свої software критерії за D38,03 має 18/17/3 software suites/builds з відкритими operator tasks,04 має 22/28/9 із T044–T050 open. Часткові HJ/USB логи не приймають accuracy/600 с.

**Альтернативи**: історичні 100 мс як default, повторний вибір Q04, переписування old evidence — відхилено. [Environment lock](../../tools/environment/environment.lock.json) і profile не змінюються; локальні tool paths мають повторний environment gate до збірки.

## RCH-02 — Час і перетворення без другого ядра

**Рішення (D69/D72)**: immutable config `LegacyDisabled|Test|Fly` у наявному ControlCore, default constructor лишається legacy; конфігурацію передає власник CapturePolicy. Operational accepted-measurement order відокремити від service-now/input-history. T=500000 мкс, включна межа age≥T. Сервіс owner номінально 1 мс; запізнення планового cutoff≤1 мс, checked LOW result≤2 мс після deadline; нормальний valid-end→checked apply≤19 мс, фізичне valid-falling→новий сталий стан/правильна PWM форма≤20 мс.

**Підстава**: нинішні control.cpp:4–40 і rc_capture.hpp:85–111 містять один core усередині capture. CapturePolicy::fault(now) у rc_capture.cpp:34–60 пересуває старий core last_event_time до now; actual falling, який прийде пізніше, може бути раніше fault-now. Для 05 input-history/service не повинні забруднювати measurement ordering; old legacy contract лишається. Candidate спершу проходить processing_complete budget, лише потім commit у core/output: поточний owner викликає processing_complete вже після on_batch.

**Обґрунтування чисел**:500 мс залишає 500 мс у погодженій 1 с для stalled path/reset/bias. Нормальний pipeline: stop5+callback/rearm1+processing10+owner1+checked apply2=19 мс у MCU clock; ще 1 мс — обраний physical transition reserve, який треба довести напряму приладом. Це acceptance allocation, не сума вже виміряних worst cases. Межі 03 не розширюються.

**Альтернативи**: T=1 с не залишає reaction запасу; T=900 мс з повільним reset не задовольняє stalled case; model clear_fault дозволив би stale calculation і не створює production recovery. Другий concurrent reducer/output task додавав би гонки.

## RCH-03 — Міграція виходу й resource lifetime

**Рішення (D70/D72)**: ролі адаптера `LegacyDisabled|FiniteBench04|Operational05`, окремі low-primed/peripheral-ready/route/health states. Сигнальні errors не technical output inhibits. Recoverable output API error → invalidation generation/epoch, checked LOW/fallback; після підтвердженого LOW до 3 автоматичних reinit-спроб, перша через≥100 мс від failure, наступні через≥100 мс від completion попередньої невдалої спроби. Це три додаткові reinit спроби після початкової операції, не нескінченний цикл. Успішний reinit на 0% → нова output epoch/fence, LOW до trustworthy valid з end>recovered_at. Exhausted — confirmedLOW/nonzero inhibited до reboot, справний supervisory цикл триває. Clock/protocol fault та LowUnconfirmed → best-effortLOW, ResetPending, припинити feed; timestamp для failure не вигадується.

**Локальні факти**: firmware/components/platform_esp32/pump_output.cpp:10–15,42–76 boot-terminal і bench_ одночасно керує LEDC init та permission. firmware/components/platform_esp32/rc_capture.cpp:406–420 OR-ить sticky input bits і примушує terminal04; startup_failed_ отруює bench readiness назавжди. GPIO fallback повертає matrix у plainGPIO: SDK esp_driver_gpio/src/gpio.c:222–227,315–344. Тому після fallback недостатньо set/update — channel routing потрібно відновити лише з duty0.

Pinned SDK esp_driver_ledc/src/ledc.c:410–450 виділяє context один раз; stop:996–1006 не звільняє timer/channel. Timer deconfigure:741–775 потребує pause й не перевіряє attached channels; configure:778–806 reset/resume. Channel config:831–895 має inner set/update/bind без propagated return. Тому після top-level config виконати окремі checked set0/update/stop0 до Ready. Не припускати destructor або нову heap allocation на кожний retry; cleanup ведеться за відомими stages.

**Lifecycle**: legacy CaptureLifecycle cleanup видаляє Worker (firmware/components/bridge_core/rc_capture.cpp:323–343; firmware/components/platform_esp32/rc_capture.cpp:75–104). Operational owner і notify timer живуть окремо від retry RMT/GPIO після handoff. Втрата входу не видаляє єдиного output/watchdog owner й не повертає GPIO0 застосунку.

**Альтернативи**: terminal latch для всіх errors обійде Fly; будь-який success, що стирає history, приховає fault; replay cached high після technical retry не є новою trustworthy командою. Не позначати cleanup successful після невдалого API.

**Уточнення власника D75/CHK017 після Plan**: кожний reinit start витрачає одну з3 спроб поточного епізоду. Reinit0/новий valid commit самі не поновлюють ліміт; його поновлює тільки checked успішне застосування нової post-fence valid-команди, включно з valid0%. Це обмежує цикл reinit0-success/apply-failure; після третьої витраченої спроби нова failure без такого apply веде у ExhaustedLow. Успішний третій reinit може чекати нову valid й закрити епізод. Sticky history та інтервал≥100 мс зберігаються; input-loss recovery не є output retry.

## RCH-04 — Реальний watchdog reset path

**Рішення (D71)**: тільки operational05: runtime TWDT100 мс, одна підписана rc-control task, `idle_core_mask=0`, `trigger_panic=true`, auto-init вимкнений. Тільки owner робить checked esp_task_wdt_reset після повного checkpoint. Weak ISR hook із сильним C-linkage symbol в IRAM викликає esp_rom_software_reset_system без GPIO/USB/heap/RTOS/logging; якщо ROM несподівано повернеться — no-feed endless fallback, не повернення в panic. IWDT500 мс, silent reboot, debugger-aware/GDB/coredump/apptrace вимкнені для acceptance.

**Первинні джерела**: локальний SDK HEAD перевірено як 880a203ae603d08a94d06aa01e0d6761958e5e9e. esp_system/include/esp_task_wdt.h:21–25 підтримує timeout_ms; task_wdt.c:475–509 викликає hook перед panic, але спершу feeds timer. task_wdt_impl_timergroup.c:120–126 задає stage0 interrupt=T,stage1 reset hold=2T. C3 clock defaultXTAL (soc/esp32c3/include/soc/clk_tree_defs.h:354–356, hal/wdt_hal_iram.c:80), prescaler20000 при 40 МГц дає 500 мкс/tick. Без ISR/feed nominal stages100+200=300 мс; послідовність stages — [C3 TRM §12.2.2,с.308](https://documentation.espressif.com/esp32-c3_technical_reference_manual_en.pdf#page=308).

**Виявлена пастка**: port/panic_handler.c:221–244 вмикає RTC WDT на 10 с і вимикає TIMG watchdogs; стандартний panic/silent reboot сам не доводить≤1 с. Ранній IWDT100 мс теж міг би зайти в цей шлях до TWDT reset stage; обрано IWDT500 мс, довше nominal TWDT300 мс. esp_rom/include/esp_rom_sys.h:20–26 називає reset digital core includingRTC, але для звичайного app restart рекомендує esp_restart. Вузький emergency hook обирається свідомо, без SDK patch; ROM symbol у C3 rom.api.ld:39, такий reset використаний SDK brownout ISR. Hook definition винести в окремий tiny TU без esp_task_wdt.h weak prototype; strong binding перевірити nm/map. Немає твердження, що IRAM hook робить весь SDK ISR cache-safe.

**Додаткова перевірка первинної документації**: [Watchdogs v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/system/wdts.html) описує task subscription/user hook та вимкнення watchdog OpenOCD під breakpoints; acceptance потребує debugger detached і fresh boot. [ESP Timer v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/system/esp_timer.html) пояснює task-dispatch serialized callback та затримки: timer notification не є proof control progress.

**Budget**: last-qualified-feed→observed reset≤350 мс, reset→physical LOW≤100 мс; разом із T500 мс —950 мс, запас 50 мс до 1 с. Це обрані перевірювані межі, не source-proven physical гарантії. Feed за Test nonzero дозволений тільки за age<T і правильного output; на/після T лише після checked LOW. За blocked API/invalidclock/feed API error немає checkpoint. No-validFly/no-reader та confirmedLOW technical exhaustion — нормальні progress states. Freeze clock не маскується однаковим raw: feed вимагає просування trusted raw від попереднього feed; MWDT має інший counter, але не незалежні power/oscillator safety domains.

**Setup/feed помилки**: до успішного init/add watchdog може не бути armed (esp_task_wdt.h:35–40,task_wdt.c:203–206); SetupBlockedLOW/nohandoff без claim no feed-reset. Після handoff feed API error викликає best-effortLOW і явний emergency ROM reset: помилка підписки не лікується очікуванням недоведеного timeout.

**Межі**: довільна відмова panic/ROM/WDT/oscillator/pad не оголошується покритою. Для неможливого timing/відсутнього reset anchor — fail/blocked, без послаблення 1 с. ROM-reset reason може виглядати як software reset; без retained breadcrumb не обіцяється watchdog cause у наступному boot. Причину доводить окрема bench fixture/прилад, USB arrival не reset anchor.

## RCH-05 — Diagnostics і build compatibility

**Рішення (D72)**: новий positional `rc05 1`, окремо від незмінного rc04/rc03-trace; grammar у contracts/operational-control.md. Розрахунок ширин полів дає консервативну верхню межу≤347B з raw SDK status/op; запас до 384B. Це sizing analysis, не formatter test. Transport latest-only/one committed/drop/version ack зберігається. Timing maxima лишаються в snapshot/bench, production не history.

Explicit `--operational-mode test|fly`, ordinary без bench — Test; `--legacy-disabled` лише явний non-operational compatibility image; rc03/pump04 flags обирають legacy family. Reject explicit operational flags+legacy/finite04, host+target options, invalid mode/trace combinations. Actual config/manifest/map мають доводити вибір, а не лише ім'я fragment.

## RCH-06 — Physical evidence method

**Рішення (D73)**: operational05 bench instrumentation не finite04: timing/stall/accuracy/stress images із тим самим Test/Fly control. Production без archive/injection. Direct Test last accepted physical falling→сталий LOW≤1 с з uncertainty guard; new valid→новий physical state≤20 мс. Latency U≤1 мс; для maximum-latency pass виміряне L+U≤limit. Це не змінює D54/M04 direct E/U acceptance rules.

Long Fly loss hold:≥600 фізичних секунд для 0/80/100%, reader active/slow/absent; потім recovery-high/low. Healthy capture regressions — окремі 600 с zero-loss/timing і короткі 128-record per-pulse windows D53–D55. Stress summary601MCU-секунда може повторно використати D62 lower-bound method, але не finite04 output cutoff. Reset/bias/probe readiness фізично pending. Один первинний журнал містить actual commands/config/source identity/очікуване-факт/U/gaps й статус; bench artifacts не є actual evidence до виконання.

## Підсумок Phase0

Технічних невибраних Q05 параметрів для Phase1 не залишено; вибори D69–D73 конкретизовані нижче в contracts/data-model/plan. На момент Phase0 G05 та physical gates були відкриті. D74 окремо закрив G05; physical Q02/Q03/Q06 лишаються відкритими з власниками, не замінені defaults. Source review/математичний sizing не є тестом firmware, build або physical pass.
