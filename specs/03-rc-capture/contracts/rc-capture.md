# Контракт захоплення та діагностики 03

Embedded boundary, без web API, network schemas чи команд насосу. Конкретні C++ імена можуть уточнюватися реалізацією зі збереженням семантики. Джерело — [spec](../spec.md), D51/D52; поля — [data-model](../data-model.md).

## Platform → переносима policy

- on_edge(EdgeEvent): GPIO observations із estimated time/level/seq/epoch; результат queue send перевіряється. Окремий latest line/sticky error зберігається незалежно від повної черги.
- on_rx_batch(RxBatch): власна копія ≤48 symbols, epoch/edge boundaries/completion metadata. IDF buffer pointer не переживає callback. Parser працює зі стандартними level/duration даними.
- poll(now_us, LineObservation): явний monotonic clock, stuck100 мс, transaction guard 12 мс, processing 10 мс, sticky errors. Timer лише notify, всі state/core calls — один owner.
- on_capture_error(reason, observed_at_us): sticky ingress signal поза чергою; повторення збільшує counter, а не стирає fault.
- Policy повертає validated measurement, explicit fault, status-only invalid/incomplete або request abort/rearm/resync. Hardware actions виконує адаптер.

### Асоціація та parser

Arm у LOW створює epoch/arm_seq. Final callback фіксує close_seq, копіює batch перед повторним використанням scratch і rearms у LOW. У нормальному batch рівно одна complete HIGH-пара відповідних rising/falling. Фінальний LOW/zero terminator нормалізується. GPIO seq/window і RMT levels/runs мають погодитися; не брати сліпо duration0 і не підбирати пару за схожим width.

Startup HIGH та stop на довгому HIGH — явні incomplete стани, не false falling. Додаткові/втрачені/неоднозначні межі — fault/discard/resync. Sanity abs((end−start)−width)≤50 мкс після перевірки порядку часу є лише додатковим rejection check, не доводить 10 мкс interval accuracy. Неоднозначний sample відкидається навіть при width у valid діапазоні.

### Узгоджені межі epoch

GPIO ISR, RMT ISR і owner використовують один короткий shared critical-section протокол для epoch/arm_seq/close_seq, latest line та recovery_requested. Копіювання batch, queue send, parsing і core calls не тримають цей lock.

При завершенні закрити old epoch і зафіксувати close_seq, перед arm перевірити LOW та відсутність pending edge interrupt, опублікувати next epoch/arm_seq і виконати arm із захистом меж. Після arm повторно перевірити LOW/sequence/pending edge. Зміна чи невизначена межа — ambiguous/discard/resync; не прив’язувати edge до довільного epoch. У нормальному потоці rearm виконується в LOW із запасом 9,9 мс. Критична ділянка має бути короткою і входити в rearm budget; жодного очікування.

Для task recovery owner спочатку встановлює recovery_requested під тим самим lock: callback перестає автоматично rearm. Owner вимикає/скидає RX поза lock, відкидає старі epoch records, синхронізується з LOW, тоді знімає прапорець і запускає новий epoch. GPIO ISR при маскуванні/повторному ввімкненні не породжує штучний rising.

Будь-який final callback завершує active guard старого epoch, включно з HIGH-overrange. Guard нового epoch починається лише від його першого спостереженого rising. Завершений overrange не отримує помилкового guard fault через 12 мс.

## Policy → ControlCore 02

Один owner використовує наявний bridge::ControlCore:

| Результат capture | Дія |
|---|---|
| Complete/matched, width 900..2100, period 18000..22000 | update_measurement(width_us, pulse_end_us), estimated falling, FIFO за end. |
| Invalid width/period, unknown period, incomplete | Status/counters/snapshot; не передавати як valid і не оновлювати last-valid. |
| Capture fault | update_capture_fault(now_us) плюс reason/counter, окремо від measurement. |
| Пізній pre-barrier result | Discard/counter; не замінювати end на now. |
| Core TimeOrderError | Capture fault/resync, без мовчазного виправлення часу. |

Fault latch має пріоритет. Accepted valid під latch може змінити calculated duty/last-valid; effective_duty_percent=0, work_permitted=false. Існуючий clear_fault()02 не експонується у production03. Reset/resync/reconnect не є arming. API core 02 не змінюється.

## Time та failure boundary

Усі edge/monitor/core times — одна шкала мкс. ISR times estimated; RMT completion/console times — metadata доставки.

Rearm normal≤1000 мкс після stop=falling+5000; validation/core≤10000 мкс після completion. HIGH/LOW threshold 100000 мкс, sticky/core delivery до age 110000 мкс. Це вимоги дизайну, не вже досягнуті результати. Active RX без final після першого rising12 мс — abort/fault у додаткові 10 мс; LOW без фронту контролює stuck monitor.

Queue overflow, saturation48, seq mismatch та API errors рахуються. На GPIO overflow ISR маскує interrupt і будить owner, той відновлює під LOW. Немає універсальної гарантії виявити кожну приховану hardware loss у довільному шумі. Init failure зберігає LOW; resource retry owner не частіше 100 мс. Reader readiness не блокує capture.

## Owner → diagnostics → USB

Один актуальний snapshot одного RC, output/core state, status/reasons/counters. Кожна заміна непрочитаного snapshot рахується. Форматування/driver calls лише diagnostics task; інші USB writers, printf/logging/per-edge records відсутні.

- Launch attempts розділені ≥100000 мкс, не більше 10 записів/с. Береться актуальний snapshot, старий серіалізований текст не зберігається для retry.
- Перед write — usb_serial_jtag_wait_tx_done(0). Busy/unavailable — skip/counter без очікування. is_connected() лише negative hint: USB SOF не доводить відкритого reader.
- Fixed buffer≤384 B, TX ring512 B, одне usb_serial_jtag_write_bytes(...,0). Native driver приймає item цілком або повертає 0. Не додавати другий record до drained state.
- Максимум один committed/in-flight record уже належить transport і не відкликається. Після reconnect він може завершитися; наступний звичайний слот містить latest state. Firmware pending history відсутня; байти в USB/OS host buffer не є контрольованою чергою snapshot.
- Record починається newline + marker rc03/sequence і закінчується newline. Consumer приймає лише повні marked lines. Disconnect може обірвати передачу; remainder не повторюється. Mock partial write рахується drop, наступний record відновлює framing початковим newline.
- Sticky faults/counters живуть у state, а не тексті. Drop не стирає помилку. Текст fault може з’явитися пізніше 110 мс; in-memory/core deadline незалежний.

Мінімальні змістові поля: seq/session, width, estimated start/end, period/status, calculated/effective duty, permission, reason bits, capture/diagnostic drops. Максимальні значення всіх полів мають укладатися 384 B — окремий boundary test. Diagnostics не має командного входу.


## Уточнення якості контракту, checklist 2026-10-29

### Класифікація та часові епізоди

Очікуваний stop на довгому HIGH — incomplete_overrange, а не несподіваний truncation. Invalid period сама не fault; окреме перевищення guard, queue loss або недостовірні межі зберігають власні capture-fault причини, зокрема для unsupported stream.

Завершений спостережений level episode ≥100000 мкс також вимагає stuck fault: новий edge не стирає історію епізоду до обробки owner. Його deadline = episode_start +110000 мкс. Якщо owner запізнився, повідомити fault із deadline_missed/threshold_at/reported_at/lateness; acceptance failed, не backdated pass.

При маскуванні ISR після overflow observation_continuous=false. Попередні last_change/time не видаються за доказ незмінного фізичного рівня протягом невидимого інтервалу. Recovery фіксує observation_started_us на re-enable, читає поточний level як initial observation без штучного edge, відкидає partial pulse/period до resync; fault/last-valid зберігаються.

### Одиниці counters та version acknowledgment

| Counter | Одна одиниця |
|---|---|
| overwritten_snapshots | Одна непрочитана версія замінена новою до успішної передачі transport. |
| busy_slots | Один eligible слот ≥100 мс пропущено до write через busy/unavailable transport. |
| write_drops | Одна здійснена write-спроба дала zero/partial/error; не дублювати busy_slots. |
| capture_drops | Одна втрачена capture event/batch із зазначенням виду; не змішувати з diagnostic records. |

Snapshot version споживається при успішному повному handoff transport; snapshot, який оновився паралельно з formatting/write, залишається pending. Ack стосується тільки прочитаної версії. Повторне читання/version check без заміни не збільшує overwrite. Saturation sticky; reset лише новою session/test reset, не reader reconnect.

Max-payload bench records мають 384 B разом із framing, актуальні поля та дозволені trailing spaces до кінцевого newline; жодних фальшивих capture values чи нового command input. Spacing ≥100000 мкс лишається тим самим. За backpressure потрібні фактичні attempts/bytes/skip/drop counters, а не твердження про неможливе повне передавання.

## Обсяг доказів, CHK025

D53 / CHK025-A (2026-10-29): для завершення 03 точні межі періоду 18/22 мс, точні граничні ширини та складні malformed-послідовності покриваються host fixtures. Фізично обов’язкові HJ Tester на 50 Гц у доступному діапазоні, width/interval accuracy, zero loss під diagnostic load, stuck HIGH/LOW та rearm/processing/fault budgets. Окремий генератор цих точних stimuli не блокує 03; host не підміняє зазначені обов’язкові фізичні докази. Q03/H02/H03 і receiver gate зберігаються.

## Контракт acceptance, CHK026-B

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

U є характеристикою методики порівняння, а не додатковим дозволом змінити width/period validation або timestamp. Whole-test pass потребує виконання кожного застосовного per-pulse/per-interval критерію та решти zero-loss/latency критеріїв; це правило саме по собі не приймає ітерацію.

## Bench-only accuracy contract (D55)

D55 / C1-A (2026-10-29): точність перевіряється в коротких вікнах із bounded RAM trace всіх імпульсів цього вікна та вивантаженням після нього. Окремі 600-секундні тести перевіряють втрати й часові бюджети під max diagnostics. Допуски 5/10 мкс та U≤1 мкс окремо за D54 незмінні; середнє не замінює per-pulse/interval порівняння. Production diagnostics не стає історією вимірювань.

Live capture phase не передає per-pulse records у USB: portable collector отримує всі послідовні candidates в owner, максимум 128×48 B+256 B metadata, без overwrite/I/O. Capacity128 означає freeze на останньому record; record_count/first/last seq і window bounds фіксуються. Invalid/unknown status не приховується, loss/order mismatch не відкидаються з вибірки.

У frozen phase owner зупиняє capture; outputLOW/permissionfalse/latch лишаються. Export bench archive використовує той самий sole USB writer, zero-wait/drain gate, ≤384 B/frame і ≥100 мс між frames, але має marker rc03-trace та явний capture_active=false. Archive складається з HEADER, indexed records, END із count/window identity; повні незмінні цикли можуть повторюватися для reader, що повернувся. Partial/змішані cycles не evidence; production mode ніколи не експортує archive/історію.

Reference format для offline tool — CSV pulse_start_ns,pulse_end_ns з raw analyzer provenance. Operator встановлює відповідність від першого відомого rising після LOW; count/order/window checks перед E. RMT width порівнюється окремо від GPIO end−start. Правило D54 застосовується до кожного correct sample/adjacent rising interval у кожному короткому вікні; воно не поширює claim accuracy на всі pulses окремого 600 с run.

## Уточнення входу D56,2026-10-29

Після кожного успішного створення RMT RX-каналу адаптер задає GPIO3 GPIO_FLOATING, вимикаючи внутрішні pull-up/pull-down, які інакше залишає SDK. Виклик перед callbacks/enable, результат перевіряється; помилка видаляє створений канал і повертає init failure у наявний fault/retry lifecycle. Зовнішній дільник задає bias. Це прямо погоджена програмна зміна; фактичні HJ рівні/фронти, номінали дільника й Q03/H02/H03 залишаються фізично pending. GPIO0, карта/пороги capture, core02/arming не змінюються.
