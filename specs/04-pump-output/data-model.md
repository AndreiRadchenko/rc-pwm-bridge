# Модель даних 04

Дата: 2026-10-30. Дизайн, не реалізовані типи. [Spec](spec.md), [контракт](contracts/pump-output.md), D59/D60. Portable заголовки strict C++11 не включають GPIO/FreeRTOS/IDF; конкретні C++ імена допускають технічне уточнення без зміни семантики.

## Незмінний BenchConfig

| Поле | Тип / правило |
|---|---|
| mode | Normal або Bench; default Normal, compile-time input |
| scenario | Waveform, Stress або Accuracy тільки за Bench |
| initial_low_us | uint64,5 000 000 |
| plateau_us | uint64,10 000 000 або100 000 000 за сценарієм |
| points | Незмінний масив6 uint8:0,25,40,50,80,100 |
| trace_step | uint8,0..5, тільки Accuracy; не змінює points |
| build/config identity | SHA256 source/config у build manifest; короткий fingerprint у header/diagnostics |

Значення не приймаються з RC/USB/NVS. Host або несумісні build flags мають відхиляти bench inputs, не ігнорувати. Тривалості/арифметичні переповнення перевіряються до старту.

## BootContext і ownership

Одна BootContext на app boot: reset reason як informational integer, startup_ready, startup_had_failure, capture_ready, ownership_handed_off, immutable config. Reset reason ніколи не є дозвільним фільтром за D59. Наявний startup supervisor має до5 спроб D15; будь-яка зафіксована невдача робить bench eligibility false в цьому boot, навіть після успішного retry. Програмна діагностика/capture recovery може продовжуватися з LOW.

До handoff тільки app/startup має право LOW операцій; після нього тільки capture/control owner виконує output policy/adapter. `capture.start` залишається idempotent; повторний app poll не повторює handoff або запуск.

## OutputSession і переходи

| Стан | Запит / дозвіл | Перехід |
|---|---|---|
| Disabled | LOW,false | Справний Normal; fault→Fault/LOW, без bench permission |
| InitialLow | LOW,false | Перший owner poll фіксує t0. До5с capture/setup/fault checks; на deadline readiness→Step0, інакше Fault |
| Step0..Step5 | points[index],true | Один перехід на абсолютному deadline; overdue>1мс→Fault, після Step5→Complete |
| Complete | LOW,false | Terminal до наступного boot |
| Fault | LOW,false | Terminal; resync/valid/retry/reader не змінюють |
| EvidenceFrozen | LOW,false | Accuracy capture freeze/архів; terminal до наступного boot |

Step index uint8,0..5; t0/deadline/last_now uint64 мкс. `request_generation` uint64; новий nonzero request не може мати generation із минулого стану після terminal fault. Переповнення generation/time — fault, не wrap-повтор. `capture_fault_bits` uint32 зберігає чинні03 причини. Поле `bench_permission` ніколи не змінює `ControlSnapshot.work_permitted`/effective duty.

Новий app boot створює всі ці об'єкти заново та може почати нову скінченну сесію. Resume step або persistent arming state немає. У firmware не додається reset command або auto reset.

## OutputRequest та результат adapter

Request: generation uint64, percent uint8/validated0..100, requested_at/deadline uint64, permission bool, terminal reason mask uint8. Fault/permissionfalse згортають request до LOW до I/O. Invalid percent відхиляється до mapping counts.

ApplyResult: generation, requested mode, observed API-result mode (Unknown/Low/Pwm/High), operation kind (Init/Set/Update/Stop/GpioLow/GpioDirection/Cleanup), raw SDK status int32 для record кожної операції; first/fallback summary додатково у header, before/after uint64. Слово applied означає лише успішні API, не вимірювання GPIO. При частковій відмові requested/apply mode можуть різнитися; останній успіх не стирає першу помилку.

Output reason bits uint8, sticky протягом boot:

| Біт | Значення |
|---|---|
| 0x01 | InitFailure |
| 0x02 | InvalidRequest |
| 0x04 | CaptureFault |
| 0x08 | ClockError |
| 0x10 | ApplyApiError |
| 0x20 | LowUnconfirmed |
| 0x40 | DeadlineLate |
| 0x80 | EvidenceOverflow |

Completion і EvidenceFrozen — state, не помилковий fault. Стан LOW після справного freeze не очищує capture latch; наявний fault має пріоритет над freeze/completion.

## DiagnosticSnapshot04

Успадковує startup/capture snapshot03 і counters; додає mode, phase, requested duty, bench permission, API-applied mode та output reason bits. Core calculated/effective/permission не підміняються. `LocalDiagnostics.publish_control` публікує узгоджену пару capture/output від одного owner, а не дві конкурентні версії. До handoff startup snapshots містять Disabled/LOW metadata.

Немає IDF типів у portable OutputSnapshot; platform snapshot може містити portable об'єкти. Повний snapshot≤384B — static assertion; serialized `rc04`≤384B із framing — окремий formatter boundary test. Enum діапазони/masks перевірені, непридатний status не форматується як довільна довга величина. Bounded transport/ack/counters03 незмінні.

## OutputEvidence тільки bench

До64×40B records та header≤128B, разом≤2688B. Структура record визначена в [контракті](contracts/pump-output.md#bounded-вимірювальні-записи-та-зіставлення-часу). Header зберігає також first/fallback SDK error та operation, counts/identity/freeze/overflow. Не зберігає per-edge output history; немає heap/overwrite/USB під час запису owner. Export читає тільки frozen records.

Archive format version1: `rc04-out H`, `R`, `E`; усі числа hexadecimal. H: build fingerprint, scenario, boot label, count, first/last event seq, frozen/overflow, first_error_operation/status, fallback_attempted та fallback_error_operation/status. За відсутності first_error: operation0/status0; fallback_attempted0 відрізняє «не було fallback» від його success зі status0. R: index, seq, kind, duty, apply_state, reasons, sdk_status (32-bit two’s-complement hex), before, after, deadline_or_fault. E: count та FNV-1a64 записів у порядку index (цілі у canonical big-endian widths4,1,1,1,1,4,8,8,8; canonical record36B, native alignment≤40B). FNV offset basis0xcbf29ce484222325, prime0x100000001b3, modulo2^64. Hash обчислюється bounded після freeze поза owner, без crypto dependency; це integrity check, не криптографічне твердження. У кожному frame поточні sticky capture/output summary поза immutable archive digest. HEADER/R/END identity/count/order мають погоджуватися; partial/mixed cycles — blocked.

Boot label/start time не гарантує глобальної унікальності: оператор зберігає кожен boot в окремий raw-файл із SHA256 та provenance, як у03. Capture accuracy archive128×48B+header≤6400B зберігає чинний D55 формат. У bench Accuracy обидва bounded collectors разом≤9088B; normal їх не містить. Actual sizeof/static RAM/map та stack high-water перевіряються при реалізації, не оголошені виміряними тут.

## Дані прийняття

Для кожного випадку: firmware/config/source hashes, profile/unit/driver identity, сценарій і boot, фізичний стан load disconnected, схема живлення/ground/bias/conditioning, прилад/threshold/timebase/U, raw traces та нормалізація, before/after counters, declared limits, measured E/latency/voltage, pass/fail/blocked/not_run і owner. Host/build/physical records розділені; старі01–03 measurements не видаються за нові04.

## CaptureWindowEvidence — тільки stress, D61

Один статичний об’єкт≤512B, без heap/overwrite/per-pulse records. Разом з OutputEvidence stress≤3200B; accuracy trace03 у stress відсутній. Normal/waveform/accuracy не містять цього collector. Стани waiting → collecting → closing → frozen; fault/неповне вікно завершує frozen з incomplete, не створює нової сесії. Capture працює далі; frozen summary не оновлюється поточними лічильниками та живе до наступного boot.

Зберігаються version1/build/scenario/boot identity; start_us=t0+5000000, end_us=t0+605000000, фактичні checkpoint_start_at_us/checkpoint_end_at_us/finalized_at_us; ingress watermark/closure status; global baseline/end snapshots та window-local counts/maxima. Global snapshots беруться одним owner на першому service на/після start та під час closure; це допоміжні actual-time snapshots, їх різниця не є window count. Облік належних samples виконується за estimated end_us, включно з обробкою після end; алгоритм — контракт.

Window counts uint32 у фіксованому порядку: candidates, valid, invalid, incomplete, dropped, faults, duplicate, reordered. Не всі діагностовані втрати мають точний end_us: невіднесена подія позначає incomplete, а не губиться з підрахунку. first_seq,last_seq,first_epoch,last_epoch — uint32; first_end_us,last_end_us — uint64, із явним has_samples. Maxima uint64: rearm_us,processing_us,fault_delivery_us; rearm/processing стосуються належних samples, fault-delivery — діагностованих належних faults. has_fault_delivery відрізняє відсутність такого вимірювання від0. Global snapshots містять сім чинних03 uint32 counters у порядку batches,candidates,valid,invalid,dropped,faults,duplicate_edges. Saturation та будь-яка неоднозначність/втрата/порушення бюджету зберігаються flags і забороняють pass. Ширини counters03 не змінюються, modulo seq/epoch03 не підміняються необмеженими числами.

Wire export `rc04-win` version1: H/B/W/T/E, числа hexadecimal. Кожен frame містить version/build/boot identity та поточні sticky fault/postfreeze_invalid summary. H містить усі часові bounds/checkpoint times, watermark, flags, has_start_checkpoint/has_end_checkpoint/has_samples/has_fault_delivery, count3. B містить baseline та end сімок global counters; W — вісім window counts, чотири seq/epoch поля і два end timestamps; T — три maxima. E містить count3 і FNV-1a64 canonical B/W/T у цьому порядку: B14×uint32, W12×uint32+2×uint64, T3×uint64, big-endian без padding, разом144B, FNV constants як rc04-out. Header/identity/bounds перевіряються додатково; digest не є криптографічним доказом. За has_samples0 boundary поля0, counts/flags усе одно явні. За has_fault_delivery0 fault_delivery_us0 не видається за виміряну нульову затримку. Немає checkpoint через ранній fault — відповідний has_checkpoint0, час/поля0 та incomplete; нулі не є фактичним snapshot.

Frozen частина незмінна, окремий sticky postfreeze_invalid піднімається при виявленні пізнішої події зі start_us вікна≤sample.end_us<end_us вікна, яка мала бути врахована, або порушеного closure barrier. Такий run failed навіть за правильного digest. Звичайні пізні події поза закритим вікном не дописуються в нього. При новому boot попередній RAM summary втрачається; змішані boots/partial cycles не приймаються.

Wire flags uint32:0x01 Incomplete,0x02 Saturated,0x04 ClosureLate,0x08 AssociationUnknown,0x10 CaptureFault,0x20 LossOrOrderError,0x40 BudgetMiss,0x80 Frozen. Presence fields/postfreeze_invalid —0 або1. Інші bits/невалідні presence codes відхиляються. Pass вимагає Frozen без failure bits, обох checkpoint presence та однозначного source matching; відсутність faults не видається за виміряний fault_delivery0. Overflow/невизначена причина не маскуються reserved bit.

## Стабільні коди протоколу04

Усі числа wire-формату hexadecimal. mode:0 Normal,1 Bench. scenario:0 Normal,1 Waveform,2 Stress,3 Accuracy. phase:0 Disabled,1 InitialLow,2 Step0,3 Step1,4 Step2,5 Step3,6 Step4,7 Step5,8 Complete,9 Fault,a EvidenceFrozen. apply_state:0 Unknown,1 Low,2 Pwm,3 High. Core o/a залишаються0/0 за04.

Record kind/operation:0 None (тільки «не задано» у header),1 SessionStart,2 Init,3 Set,4 Update,5 Stop,6 GpioLow,7 GpioDirection,8 Cleanup,9 Fault,a Complete,b EvidenceFrozen. Додаткові kinds вимагають явної версії/схеми, а не вільного internal enum ordinal. Generation не є wire-kind. Неавторизовані enum values відхиляються; всі phase/apply_state/mode вкладаються у задекларовані розміри rc04.

Кожний API record зберігає sdk_status як32bit two’s-complement hex; status0 означає ESP_OK. Fault/terminal події без SDK-виклику мають status0, але kind і reason_bits не дозволяють трактувати їх як успішну апаратну операцію. Header summary не замінює per-operation records. За overflow неповний архів не приймається, а LOW handling не залежить від можливості append.

Initial_levels/acquisition bounds/gap metadata для фізичних edges є обов’язковими умовами static endpoint evidence; порожній CSV без них — blocked. Усередині заданого observable window edges мають чергувати рівні від початкового state. Немає canonical clock offset між MCU та analyzer за замовчуванням.
