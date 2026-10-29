# Дослідження: 03 RC capture — попередній результат

Дата 2026-10-29. **Оновлення D51:** hardware timestamp і межа 3 мс більше не є чинними вимогами. Поточний напрямок — RMT width + оцінені GPIO ISR timestamps і окремий monitor із порогом 100 мс. Технічний дизайн завершено в D52/Plan; чинні висновки — RCH-10–13 наприкінці цього документа. RCH-01–08 нижче збережено як історію пошуку за попередніми вимогами; їхні висновки про блокування не визначають поточний статус. Це дослідження не є реалізацією, target build або фізичним вимірюванням.

## RCH-01: мітки фронтів

**Потреба:** D40/FR-001 — апаратні `pulse_start_us` і `pulse_end_us` в одній монотонній шкалі, відмінні від часу обробки; ширина — їх різниця. 02 приймає `pulse_end_us` як час валідного sample.

**Доказ:** pinned ESP-IDF v5.5.6, `build/scopa01-idf-sdk/components/esp_driver_rmt/include/driver/rmt_types.h` (`rmt_rx_done_event_data_t`) містить буфер символів, кількість та `is_last`, без timestamp. `components/hal/include/hal/rmt_types.h` визначає символи як тривалості й рівні. [Документація Espressif v5.5.6 для C3](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/rmt.html) описує 15-бітні тривалості. З цих даних сам RMT callback не надає прив'язки фронту до глобального monotonic clock.

**Альтернативи:** GPIO ISR/`gptimer_get_raw_count` фіксує час програмної обробки, а не hardware latch фронту, тому не задовольняє D40 сам по собі; додатковий hardware latch/синхронізація або інша архітектура потребує конкретного підтвердження можливості й сумісності з C3. Загальний GPTimer `get_captured_count` не підтверджує конкретного маршруту фронту в C3: `components/esp_driver_gptimer/CMakeLists.txt` вмикає ETM за `CONFIG_SOC_TIMER_SUPPORT_ETM`, а C3 capability не заявляє відповідну підтримку. Не доведено ні неможливість будь-якого рішення, ні працездатність запропонованого. Вимогу змінювати без згоди користувача не можна.

## RCH-02: RX, межі та пропуски

**Потреба:** 50 Гц, 18000–22000 мкс між rising edges, width 900–2100 мкс, stuck HIGH ≤3 мс, LOW ≤100 мс, 0 втрачених коректних імпульсів за 10 хв під навантаженням.

[Документація ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/rmt.html) описує поріг `signal_range_max_ns` для обох рівнів, stop/rearm і truncation при недостатньому буфері. `components/hal/esp32c3/include/hal/rmt_ll.h` задає idle максимум 32767 ticks; при 1 МГц це 32,767 мс. Поріг 3 мс завершував би транзакцію на звичайному LOW (приблизно 15,9–21,1 мс), а поріг вище звичайного LOW сам не забезпечує 3 мс для HIGH. При 1 МГц поріг 100 мс не поміщається в idle counter. C3 має RX ping-pong, але це не є доказом безперервного захоплення в кожному сценарії.

**Альтернативи:** окремий контроль рівня/deadline і асинхронна partial RX або rearm можливі лише після обґрунтування часових меж, ініціалізації та втрат; polling 01 у `firmware/main/app_main.cpp` кожні 10 мс недостатній як гарантія 3 мс. Конкретні resolution/filter/buffer/rearm не затверджені, доки timestamp і виявлення втрат не мають доведеної конструкції. Число 0 втрат є критерієм прийняття, не результатом вимірювання.

## RCH-03: overflow

У `components/esp_driver_rmt/src/rmt_rx.c` виявлені debug-повідомлення про truncated symbols, але RX event не має окремого overflow flag. Заповнений до межі буфер сам по собі не відрізняє exact-fit від truncated; перехід на capture fault за D46 вимагає надійного доказового механізму або обґрунтованої зміни драйвера. Не стверджувати, що callback автоматично сигналізує overflow. Деталі bounded ISR та передачі буферів залежать від вибраної конструкції.

## RCH-04: інтеграція і діагностика

02 має host 8/8 і target build, але `app_main` не викликає `ControlCore` ([доказ](../02-control-core/verification/acceptance.md)). 03 має інтегрувати його без operational output: перевірити width і period до виклику accepted measurement, передати `pulse_end_us`; invalid period не передається як valid, capture fault передається окремо. Діагностика 01 D22 не реалізує автоматично D43 (актуальний snapshot після reconnect і лічильники втрат); потрібна окрема зміна та перевірки неблокування/обліку.

## RCH-05: фізичні gates

HJ Tester заявляє 50/125/250 Гц і живлення 5–6 В; у 03 підтримується лише 50 Гц. Схему перетворення рівня, GPIO-напруги, живлення й прилад не підтверджено; Q03 блокує підключення до підтвердження. H02/H03 01 відкриті; D48 відкладає перевірку конкретного RC-приймача до його використання, а не скасовує інші gates. 03 не використовує насос. Фізичні tolerance/zero-loss/deadline acceptance ще не виконані.

## RCH-06: додатковий пошук у TRM — 2026-10-29

**Рішення:** вимогу D40 збережено за D50. Стандартний GPIO→TIMG/SYSTIMER latch не підтверджено для C3; це не доказ неможливості всіх апаратних схем.

**Першоджерело:** [ESP32-C3 TRM v1.4](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf), перевірено 2026-10-29. §11.2.2, друкована стор. 289: latch TIMG запускається записом TIMG_T0UPDATE_REG. §10.5.1, стор. 274: читання SYSTIMER також потребує update. GPIO matrix §5.11 маршрутизує peripheral input, але в перевірених таблицях/описах не знайдено GPIO→timer edge latch. C3 capability у pinned SDK не містить MCPWM/ETM capture; PCNT не слід припускати як ресурс C3.

**Причина:** назва GPTimer `get_captured_count` і наявність апаратного лічильника не означає, що фронт GPIO захоплює його без CPU. `timer_ll_trigger_soft_capture` прямо запускає програмне захоплення. Нова документація RMT (не pinned SDK) згадує GPIO interrupt для часу початку — це програмний anchor, а не прийнята hardware timestamp.

## RCH-07: що вдалося звузити для RMT

**Кандидат, не прийнятий дизайн:** 1 МГц, один tick 1 мкс; idle stop 30 мс у межі 32767 ticks та вище максимального LOW коректного потоку 22−0,9=21,1 мс. Glitch filter вимкнений, щоб не приховати invalid короткі імпульси; необхідність/межі фізичного noise filtering залежать від electrical interface. RX hardware має 48 symbols на блок, ping-pong половини по 24 symbols. Частковий RX дозволяє довгу транзакцію, але callback надходить за наповненням, а не на кожному фронті. У звичайному потоці 50 Гц очікування 24 символів може тривати близько 0,48 с, тому воно не є шляхом для повідомлення HIGH ≤3 мс або LOW ≤100 мс.

`rmt_receive` дозволений у ISR; final callback викликається після переходу FSM до enabled, тому rearm із final callback дозволений. У partial callback транзакція ще триває. Жоден варіант сам по собі не доводить нульову dead time.

Консервативний fault при заповненому final non-partial буфері може відкинути exact-fit, але не є доказом виявлення кожного hardware overrun. Для partial RX повний фрагмент — нормальний випадок; такий критерій не можна застосувати як універсальний overflow detector. У pinned C3 драйвері RX_THRES/RX_DONE і debug truncation не утворюють явний per-event loss flag. Потрібні окремі bound/detection proofs. Критерій 0 втрат за 10 хв лишається тестовою вимогою, не універсальною гарантією довільного malformed потоку.

## RCH-08: кандидат hardware sampling через I2S raw PDM RX

**Знайдена можливість:** C3 capability `SOC_I2S_SUPPORTS_PDM_RX=1`, `SOC_I2S_PDM_MAX_RX_LINES=1` у pinned `components/soc/esp32c3/include/soc/soc_caps.h`. GPIO3 може подавати вхід одночасно в RMT та I2S DIN через GPIO matrix; новий GPIO не призначено. `i2s_pdm_rx_set_gpio` у pinned `components/esp_driver_i2s/i2s_pdm.c` допускає `clk=-1`, тож зовнішній clock pin не є безумовно потрібним у master mode. Працездатність fanout/config разом ще не перевірено.

[Офіційна документація I2S C3 v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/i2s.html) описує raw PDM RX і 16-бітні одиниці буфера; це апаратна дискретизація, а не час callback. Кандидат: знайти rising/falling transitions у послідовності hardware samples та обчислювати edge time за номером відліку. RMT залишається для width/контрольного зіставлення; I2S не обрано заміною обов'язкового RMT RX.

**Важливе уточнення:** вимога спільної монотонної шкали сама по собі не вимагає epoch саме `esp_timer`. Потенційно єдину capture-шкалу можна визначити hardware sample index, якщо нею ж визначені current time/deadlines та вхідний час ядра. Це кандидат архітектури, не доведений hardware latch і не прийнята інтерпретація D40. Неперервність, масштаб, затримка надання current time й поведінка після зупинки/втрат повинні бути доведені; проста сума duration чи довільний software anchor цього не робить.

**Proof obligations перед завершенням дизайну:**

1. Підтвердити точний порядок бітів raw PDM, slot packing, реальну частоту sample для C3; у `i2s_pdm_rx_calculate_clock` raw BCLK = `sample_rate_hz * 2`, тому не можна прирівняти параметр API до часової ціни біта без перевірки.
2. Визначити початок hardware sample epoch, непреривний індекс і поведінку FIFO/GDMA на start/restart; жодного перенесення callback latency в edge timestamp.
3. Обґрунтувати current time/deadline в тій самій шкалі, у тому числі HIGH при startup, LOW без фронтів і зупинений capture clock. Доставка DMA + черга + decoder повинна вкладатися у 3/100 мс; частота sampling сама цього не доводить.
4. Визначити кількість/розмір DMA буферів, bounded ISR/copy і overrun detection. I2S має `on_recv_q_ovf`, але це втрата програмної черги, не автоматичне доведення всіх hardware losses.
5. Зіставити hardware-sampled фронти з RMT подіями без неоднозначності; розбіжність/втрата — явний capture fault і resync D46. Точність width ≤5 мкс, rising intervals ≤10 мкс і нуль втрат/10 хв залишаються acceptance gates.
6. C3 має одну PDM RX line: сумісність із двома незалежними входами 07 не заявляється. Будь-який додатковий clock/pin/resource потребує окремої перевірки профілю.

**Висновок:** знайдено суттєво інший підтверджений апаратний ресурс, але не готове рішення всіх D40/FR-001. Phase 1 і dependent implementation ще blocked; зовнішнє обладнання/нову плату не обрано. SPI slave також переглянуто: він потребує CS і clock/handshake та не надає готового безрозривного timebase, тому не є обґрунтованою простішою альтернативою.

## RCH-09: спрощений напрямок на основі прикладів (D51)

Розглянуто вихідний код, а не лише README:

- [CyberBoat, pinned receiver](https://github.com/slopeztech/CyberBoat/blob/d85d617ea6dc1538abd75682983217f0aa5fc4d3/components/cyberboat_driver_pwm_receiver/src/pwm_receiver.c): GPIO ISR читає монотонний час, зберігає rising і обчислює width на falling; лише valid width оновлює last-valid. Беремо принцип програмних міток та розділення activity/valid; у нашому проєкті ширина лишається RMT і додатково перевіряється period.
- [rc_functions_controller, pinned RMT input](https://github.com/rengels/rc_functions_controller/blob/aaad9b9b8be3178b406288a21c17b92b4a8dd0b2/src/input/input_ppm.cpp): RMT durations, callback→черга→обробка/rearm. Беремо поділ capture/worker; не переносимо ігнорування результату queue send, платформні залежності або невизначені втрати при rearm.
- [ESP32_PWM_MOSFET_Trigger_Switch, pinned capture](https://github.com/fazeelahmad72/ESP32_PWM_MOSFET_Trigger_Switch/blob/823056d90eab2556fbb565f4f4a95ab5944a068d/main/pwm_capture.c): MCPWM hardware capture для width/period. Пряма заміна API на C3 не підходить: цього периферійного блока немає; таймерний monitor та розділення decode корисні як загальні підходи.

**Рішення вимог:** D51 дозволяє програмні timestamps, не вимагає hardware latch або універсального детектора прихованих втрат. Спільний поріг незмінного рівня HIGH/LOW — 100 мс від останньої зміни або запуску. Callback RMT і друк не є джерелом edge time; RMT width і різниця GPIO timestamp мають різне походження.

**Подальше рішення:** RX/rearm, асоціацію GPIO/RMT, черги й бюджет визначено RCH-10–13 та D52. Частіша обробка не доводить відсутність втрат; програмний timestamp не доводить допуск 10 мкс. Допуски 5/10 мкс і 0 втрат за 10 хв залишаються окремими фізичними критеріями. Приклади ще потребують адаптації до native ESP-IDF v5.5.6.


## RCH-10: чинний вибір RMT transaction/rearm (D52)

**Decision:** APB, 1 МГц; filter0; stop5 мс; mem_block_symbols48, buffer192B, non-DMA/non-partial. Один нормальний HIGH на транзакцію. Final callback копіює bounded batch і викликає rmt_receive перед очікуванням worker. Rearm у LOW, budget≤1 мс після stop.

**Rationale:** [C3 TRM, §33.3.5.1](https://documentation.espressif.com/esp32-c3_technical_reference_manual_en.pdf) описує початок RX counting із першої зміни рівня після enable. Тому arm у LOW не потребує циклічного initial-idle restart. Min valid LOW15,9 мс дає запас 9,9 мс після stop5 мс і rearmbudget1 мс. 1 tick1 мкс значно менший за width tolerance5 мкс, але фактичну точність це не доводить.

Pinned SDK commit880a203ae603d08a94d06aa01e0d6761958e5e9e, локальні первинні джерела:
- components/esp_driver_rmt/include/driver/rmt_rx.h:80–85: receive nonblocking/ISR allowed, cache-off потребує RMT_RECV_FUNC_IN_IRAM.
- components/esp_driver_rmt/src/rmt_rx.c:668–678: FSM ENABLE до final callback; rearm дозволений.
- rmt_rx.c:292–299: input/matrix routing сумісні з GPIO input ISR, який ставиться після створення каналу; не reset pin після routing.
- rmt_rx.c:394–439: обчислення idle/filter та disable filter при 0.
- rmt_rx.c:636–641,721–726: truncation не дає універсального per-event flag; saturation48symbols консервативно fault.
- components/soc/esp32c3/include/soc/soc_caps.h:251–259: 48words, RX ping-pong, APB.
- components/esp_driver_rmt/Kconfig: RX_ISR_CACHE_SAFE та RECV_FUNC_IN_IRAM; не покладатися на deprecated загальний RMT_ISR_IRAM_SAFE.

**Alternatives:** long continuous RX/partial callback має велику batch latency; arm з worker залишає gap залежним від scheduling/diagnostics. Hardware latch/I2S більше не потрібні D51; GPIO-only не відповідає поточному RMT scope. [Офіційний RMT guide v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/rmt.html) — API reference, не доказ performance.

## RCH-11: association, ресурсні межі й час

**Decision:** 64 EdgeEvents, 8 RxBatches, epoch/sequence windows, єдина повна HIGH-пара; додаткова різниця GPIO duration/RMT width≤50 мкс. API errors/queue drop/saturation/недостовірні межі — sticky fault поза чергою, discard/resync. Initial HIGH та очікуваний HIGH stop5 мс — incomplete, end не вигадується. Guard12 мс від першого rising обмежує транзакцію без idle; processing≤10 мс від completion. GPIO overflow маскує ISR до owner recovery у LOW.

**Rationale:** width matching без seq/epoch може пов’язати повторювані імпульси з чужим часом. FIFO pointer lifetime обмежений callback, тому копіювання перед rearm обов’язкове. Saturation може бути exact-fit, але conservative fault прийнятний для malformed stream; нуль втрат normal valid перевіряється окремо. Додатковий guard дає окрему ресурсну причину fault, а invalid period сам по собі latch не встановлює.

**Decision:** estimated ISR timestamps esp_timer_get_time(), один clock; 1 мс task-dispatch notify owner priority10, diagnostic priority1. Поріг 100 мс + delivery budget10 мс. Monitor читає coherent snapshot і повторно перевіряє seq; один stuck counter increment на епізод.

**Rationale:** [ESP Timer v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/system/esp_timer.html) дозволяє get_time у ISR, але latency timer/task не є hardware гарантією. Бюджет 10 мс обраний дизайном і перевіряється під load, не успадкований від loop01. Current core02 відхиляє regressing event time: control.cpp:8–31. Один owner/fault barrier відкидає pre-barrier late samples без підміни end на now. Accepted newer samples під latch дозволені D37.

**Alternatives:** спільний callback timestamp зіпсує freshness; core calls ISR створять гонки; один лише RTOS poll10 мс без budget/evidence не доводить deadline. Operational last-valid age/cutoff залишається 05.

Shared epoch/sequence/line/recovery state має короткий critical-section протокол; stable LOW/seq/pending interrupt перевіряються навколо arm. Concurrent boundary відкидається. Final callback завжди закриває active guard; новий guard від першого rising. Це усуває гонку metadata та false guard fault після завершеного overrange.

## RCH-12: діагностика без прихованої історії USB

**Decision:** один pending latest snapshot і максимум один committed record≤384 B, TX ring512B; один writer, wait_tx_done(0) перед write(...,0), launch spacing≥100 мс. Busy/overwrite/drop рахуються, старий текст не повторюється. Newline+rc03/seq framing відновлює читання після обриву.

**Rationale:** native components/esp_driver_usb_serial_jtag/src/usb_serial_jtag.c:265–288 приймає whole ring item або 0; :291–317 дає zero-wait drained check. Include driver/usb_serial_jtag.h:95–106 прямо відрізняє USB SOF connection від відкритого serial reader. Без drain gate навіть zero-wait writer накопичить старі записи в ring. Уже committed bytes не відкликаються: після reconnect може завершитися один старий in-flight record, наступний slot — latest. OS buffer поза firmware ownership; це не обіцянка першого свіжого байта на host.

**Alternatives:** необмежений queue/retry/burst суперечить D43; delete/reinstall driver для purge не потрібний; port-open не вигадується. Mock partialwrite і physical interruption тестуються окремо, remainder не повторюється.

## RCH-13: статус дослідження та acceptance inputs

Усі blocking technical choices для Phase1 визначено D52. Числа є обраними межами реалізації до тесту, не вимірюваннями або новою політикою Q05. Phase1 artifacts: data-model, embedded contract, quickstart; Tasks/Implement не виконані.

Physical gates: conditioning/напруги/ground Q03, H02/H03 01, analyzer accuracy/sampling, фізичні fixture stimuli, each-width5 мкс, rising-interval10 мкс, rearm/processing/fault budgets, zero valid loss600 с під max diagnostic load. HJ — acceptance03; receiver gate до використання D48. Нічого з цього не позначено passed. Якщо вимірювання не вкладаються в межі — fail і виправлення або окреме рішення, не автоматичне послаблення.


## Checklist 2026-10-29: очевидні уточнення без зміни D51/D52

Expected HIGH stop відрізнено від unexpected truncation; invalid period не приховує незалежного resource fault. Stuck episode, завершений до monitor, все одно має deadline; missed deadline фіксується як failure із lateness. Masking створює observation gap, не доказ фізичної незмінності. Diagnostic overwrite/skip/drop counters отримали різні одиниці та version-specific ack. Max-load bench має 384 B record з актуальними даними й padding, без підміни capture values. Physical scope визначено D53/CHK025-A; правило прямого порівняння E з окремим U визначено D54/CHK026-B; нових результатів тестів немає.

## Рішення власника CHK025

D53 / CHK025-A (2026-10-29): для завершення 03 точні межі періоду 18/22 мс, точні граничні ширини та складні malformed-послідовності покриваються host fixtures. Фізично обов’язкові HJ Tester на 50 Гц у доступному діапазоні, width/interval accuracy, zero loss під diagnostic load, stuck HIGH/LOW та rearm/processing/fault budgets. Окремий генератор цих точних stimuli не блокує 03; host не підміняє зазначені обов’язкові фізичні докази. Q03/H02/H03 і receiver gate зберігаються.

## Рішення власника CHK026

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

Обрано B замість альтернативи E+U≤допуск. Це прийняте правило оцінки результату, не твердження про виміряне U або фізичний pass.

## C1-A / D55: доступність фізичних вимірювань

D55 / C1-A (2026-10-29): точність перевіряється в коротких вікнах із bounded RAM trace всіх імпульсів цього вікна та вивантаженням після нього. Окремі 600-секундні тести перевіряють втрати й часові бюджети під max diagnostics. Допуски 5/10 мкс та U≤1 мкс окремо за D54 незмінні; середнє не замінює per-pulse/interval порівняння. Production diagnostics не стає історією вимірювань.

Рішення: короткий RAM collector 128×≤48 B+≤256 B metadata, no I/O при capture; freeze/stop і cyclic bounded archive export після вікна. Це збирає всі відповідні вимірювання навіть без reader. Production one-latest diagnostics та окремі 600 с runs не зберігають історію. Offline comparator використовує нормалізований analyzer CSV та D54.

Числа 128/6400 B — технічні параметри Plan до реалізації й map/sizeof перевірки, не виміряна вільна RAM. Альтернатива full600 с trace не обрана: вона потребувала б іншої стратегії зберігання/streaming. Accuracy claim обмежено кожним pulse/interval кожного короткого window; ніякого average acceptance або claim на всі 600 с. Фізичних результатів немає.
