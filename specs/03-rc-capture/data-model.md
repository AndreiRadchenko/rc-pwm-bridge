# Модель даних: RC capture 03

Семантичний контракт стандартного C++11. IDF handles, RMT structs, FreeRTOS objects і GPIO calls залишаються в адаптері. Поля нижче — дизайн, не вже реалізовані структури.

## Час та ідентифікатори

- MonotonicUs: uint64_t, мікросекунди від startup clock domain; adapter перевіряє невід’ємний int64_t esp_timer_get_time(). Не змішувати UTC, RTOS ticks чи callback time з edge time. Restart створює нову session.
- EdgeSeq/RxEpoch: uint32_t, модульне порівняння в bounded window значно меншому за 2³¹ значень: 64 edges/8 batches. Sequence wrap не визначає вік сигналу. Час лишається 64-bit; перевірка end≥start перед subtraction. Session mismatch або regression — fault.
- Один clock для rising/falling, monitor і core. RMT ticks — duration, 1 tick=1 мкс, не абсолютний timestamp. Два канали в 07 мають використовувати цей самий clock; у 03 другий канал відсутній.

## Сутності

| Сутність | Поля / одиниці | Ownership і правила |
|---|---|---|
| EdgeEvent | seq, epoch, level HIGH/LOW, estimated_time_us | GPIO ISR → queue 64 → owner. Час читається на початку handler і містить ISR latency. Повторення рівня не є новим falling. |
| LineObservation | level, last_change_us, capture_started_us, latest_seq, pending error/reason/counters | Окремий узгоджений snapshot під короткою critical section. Volatile не забезпечує атомарність 64-bit на C3. Overflow черги цей стан не стирає. |
| RxBatch | epoch, arm_seq, close_seq, completed_at_us, до 48 raw symbols/96 runs, API result | Callback копіює scratch у static queue 8. Completed_at — metadata доставки, ніколи last-valid. Runs мають level/duration; zero terminator не є імпульсом. |
| PulseMeasurement | pulse_start_us, pulse_end_us, pulse_width_us, optional period_us, origin seq/epoch, provenance | Start/end estimated GPIO ISR, width hardware RMT. Start<end; width незалежна від end−start. Complete/matched перед validation, sanity difference≤50 мкс. |
| ValidationResult | valid / period_unknown / invalid_width / invalid_period / incomplete_overrange / capture_error; width_ok, period_known, period_ok | Valid лише complete/matched + width 900..2100 + period 18000..22000. Кілька invalid причин не затирають одна одну. |
| CaptureState | waiting_low / waiting_rise / receiving / resync; optional period_origin_rising_us, pending batch | Startup HIGH і HIGH stop → waiting_low. Arm у LOW → waiting_rise; rising → receiving; final → decode/next epoch. Fault → resync, дві нові rising для period. |
| FaultState | sticky reason bitset, first/last_reported_at_us, barrier time/epoch, counters | Причини: stuck HIGH/LOW, queue drop, seq/epoch/shape mismatch, saturated batch, RX/rearm API, bad clock, pairing/processing/transaction overrun. Resync не очищує core latch. |
| CaptureSnapshot | останній measurement/status, last-valid end optional, line/fault/counters, core calculated/effective duty, permission, profile/session ID | Owner публікує один актуальний snapshot. Calculated0..100 — внутрішня карта; effective 0, permission false у всіх станах 03. |
| DiagnosticState | latest snapshot/version, pending/in-flight, last_launch_us, overwritten/skipped/transport counters | Один pending і один committed record ≤384 B; окремий writer, ≥100000 мкс між спробами. |

Counters uint32_t saturating; при насиченні counter_saturated=true, історія не обнуляється. Faults/counters не скидаються читанням чи невдалим друком. Явний test reset починає нову session; production clear command у 03 немає.

## Валідація та переходи

1. Startup: valid і period ще відсутні, core disarmed, GPIO0 LOW. LOW до першого фронту контролює stuck monitor100..110 мс. HIGH без спостереженого rising ніколи не complete.
2. Period = current rising−previous credible rising. Перший rising дає unknown. Invalid period переносить origin до current rising (D49); origin не є last-valid.
3. Complete matched pulse: width і period перевіряються незалежно. Invalid/unknown зберігає last-valid. Accepted valid викликає core з RMT width та estimated falling. Core повторно перевіряє width і виконує карту 02.
4. Незмінний level age≥100000 мкс: sticky fault і core update до age 110000 мкс. Зберігаються threshold_at/reported_at/lateness. Перед fault повторно перевірити edge seq; один stuck event на епізод. Regular invalid edges змінюють line age, але не last-valid; operational freshness вирішить 05.
5. RX HIGH stop 5 мс: duration неповна, incomplete_overrange, end не вигадувати; чекати LOW. Це очікуваний overrange, не fault саме через invalid width. Інша недостовірна структура/overflow — fault/resync. Saturated final 48 symbols консервативно fault навіть при exact-fit.
6. Fault barrier: повідомити core з now, відкинути/count pending pre-barrier measurements, скинути period origin/epoch. Достовірні завершені samples можна обробити перед fault лише якщо не порушується budget. Ніколи не пересувати end до часу доставки.
7. Нові accepted valid після barrier можуть оновити last-valid під latch. Capture resync, sequence rollover і reader reconnect не дають permission і не очищують core fault. Time regression — окремий fault.

Epoch/sequence/recovery_requested захищаються тим самим коротким lock, що й LineObservation; arm boundary перевіряє stable LOW/sequence/pending interrupt. Final callback закриває guard навіть при HIGH-overrange; новий guard — від першого rising нового epoch. Деталі — контракт.

## Обмеження ресурсів

RMT scratch 192 B, 8 batch copies, 64 edges, один snapshot, один текст 384 B і bounded task stacks. Фактичні sizeof/static RAM/stack watermark записати за map і target acceptance; оцінка не є вимірюванням. Overflow не розширює heap і не блокує capture. Заміна дозволена лише для latest diagnostic snapshot і рахується.


## Уточнення checklist 2026-10-29

LineObservation додатково має observation_continuous та observation_started_us; маскування ISR робить попередній level history недостовірним для прихованого інтервалу. Recovery не вигадує edge/time, запускає новий observation interval і resync, зберігаючи capture fault та last-valid.

Owner обліковує тривалість завершених спостережених level episodes; новий edge після 100 мс не стирає обов’язок stuck fault до episode_start+110 мс. Deadline_missed/lateness — окремі поля failure evidence, а не timestamp correction.

Diagnostic counters: overwritten_snapshots, busy_slots, write_drops — різні одиниці; один слот має одну причину skip/write failure. Ack успішного handoff споживає лише прочитану version, новіша лишається pending. Повторне читання не є overwrite; counters saturating до session reset.

Incomplete_overrange після очікуваного HIGH stop не є unexpected truncation. Invalid period не latch fault сама; незалежні resource/capture errors не маскуються нею.

## Обсяг доказів, CHK025

D53 / CHK025-A (2026-10-29): для завершення 03 точні межі періоду 18/22 мс, точні граничні ширини та складні malformed-послідовності покриваються host fixtures. Фізично обов’язкові HJ Tester на 50 Гц у доступному діапазоні, width/interval accuracy, zero loss під diagnostic load, stuck HIGH/LOW та rearm/processing/fault budgets. Окремий генератор цих точних stimuli не блокує 03; host не підміняє зазначені обов’язкові фізичні докази. Q03/H02/H03 і receiver gate зберігаються.

## Поля фізичного evidence, CHK026-B

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

Evidence містить E_width_us/E_interval_us для кожного порівняння, U_width_us/U_interval_us та джерело оцінки U, raw trace/timebase/sample metadata. Ці поля належать записам перевірки; вони не фільтрують capture, не змінюють validation limits і не коригують live timestamps.

## AccuracyTraceWindow (D55, bench only)

Статичний collector 128 records, кожен serialized/struct footprint≤48 B; header≤256 B, додаткова RAM≤6400 B, no heap. Record: sample_seq/epoch, estimated start/end_us, RMT width_us, optional period_us/known flags, validation/fault flags та accepted_at_us. Record має presence/status bits: unknown/invalid/incomplete не стає вигаданим valid. Header: version/session-window identity, record_count, first/last sequence, window_start/end, capture-active/frozen flags, counters baseline/end/deltas і max latencies.

Стани: disabled (production) → waiting first candidate → collecting → frozen → capture stopped/export. Нормальний full 128 завершує вікно без overwrite; після freeze records незмінні. Capture loss/ambiguity/saturated counters у вікні роблять доказ неповним, а не дозволяють викинути sample. Preexisting latch/counters зберігаються; window deltas не є reset.

Export state містить лише index/header/end cursor та один in-flight frame, циклічно читає той самий frozen buffer без збільшення history. Archive flags capture_active=false; archive phase поза вимірювальним вікном. Наступна session — окремий операторський restart.

Reference CSV містить pulse_start_ns/pulse_end_ns у цілій ns-шкалі analyzer. Comparison report містить sample correspondence/count/order, усі E_width/E_interval, max E, окремі U, source hashes та complete/failed/blocked status. Повний archive і raw analyzer export зберігаються як physical evidence.

## Відповідність реалізованому C++ API

Після Implement: PulseMeasurement має start_us/end_us/width_us/period_us,complete/period_known/status; measurement.validation() повертає ValidationResult зі status і незалежними width_ok/period_known/period_ok, тому одночасні invalid width та period не губляться за primary status. CaptureState віддається як CaptureSnapshot.phase. Логічний FaultState згорнуто у snapshot.fault_bits, first_fault_at_us,last_fault_at_us,recovery_needed і private barrier policy; core state/last-valid — snapshot.control. Profile/session/startup поля композиції лежать у DiagnosticSnapshot, не у hardware-independent core. LineObservation належить платформі й передається poll за значенням. Core02 API незмінний.

Startup HIGH явно Incomplete/WaitingLow, без fake start/end. Clock regression встановлює ClockError і latch ядра з останнім достовірним часом для впорядкування; raw sample/last-valid не переписуються. За втраченої довіри до clock цей timestamp не є доказом фактичного deadline. Future/reversed batch metadata та missing LOW/zero terminator завершеного sample відхиляються. Queue send/copy/parser/core calls поза shared metadata lock; application ISR не allocation/I/O.

## Уточнення входу D56,2026-10-29

Після кожного успішного створення RMT RX-каналу адаптер задає GPIO3 GPIO_FLOATING, вимикаючи внутрішні pull-up/pull-down, які інакше залишає SDK. Виклик перед callbacks/enable, результат перевіряється; помилка видаляє створений канал і повертає init failure у наявний fault/retry lifecycle. Зовнішній дільник задає bias. Це прямо погоджена програмна зміна; фактичні HJ рівні/фронти, номінали дільника й Q03/H02/H03 залишаються фізично pending. GPIO0, карта/пороги capture, core02/arming не змінюються.
