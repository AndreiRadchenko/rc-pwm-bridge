# Операторська процедура 03

**Підготовлено, не виконано.** Цей документ не дозволяє flash, підключення чи роботу з навантаженням. Виконувати лише за окремим дорученням після gates T043. Стан/результати записувати в [hardware.md](hardware.md), не закривати checkbox за самим існуванням процедури.

## Gates та підготовка

1. Звірити конкретну SuperMini/profile c3-supermini-pinout-01, H02/H03 evidence01, bias GPIO0, живлення3V3/GND. Режим одночасного external3V3+USB VBUS не вважати перевіреним. MD20A й насос від’єднати; DIR не має GPIO.
2. Після D56 у firmware GPIO3 не має внутрішніх підтяжок, рівень задає зовнішній conditioning/нижнє плече дільника. До з’єднання HJ із GPIO3 виміряти вихід після conditioning: схема, рівні HIGH/LOW, ground/protection, відсутність перевищення погоджених3,3В. Намір знизити рівень не є доказом Q03. Невідоме — blocked.
3. Analyzer на стороні GPIO3 після conditioning, додатковий канал на GPIO0 для LOW. Записати модель, firmware/serial, timebase specification/accuracy, thresholds, sample rate≥10MS/s (крок≤0,1мкс), схему/довжину ground. Обґрунтувати U_width та U_interval кожне≤1мкс, для interval до22мс. Сам sample rate не є повним U; невідоме/завеликеU — blocked.
4. Записати Git/source/config/ELF/bin SHA-256, SDK/tools, profile hash із build.md/software-evidence.json. Збирати власні нові directories командами quickstart. Не підміняти старі hashes новим кодом.
5. Лише після окремого доручення на flash: підготувати pinned shell згідно environment.lock.json і replay-evidence.json. У цьому shell вручну виконати `idf.py -C firmware -B build/c3-03-usb-trace -p <перевірений порт> flash` для accuracy; для600с — directory c3-03-usb-stress. Це приклади ручних команд, тут не виконані. Не застосовувати erase-flash/reset до чужих даних автоматично. Окремо перевірити GPIO0 LOW та effective0/permissionfalse; boot/reset evidence не замінюється app LOW.

## P1: accuracy window та часові бюджети

Використати accuracy+stress build. HJ лише50Гц;125H/250H не підтримуються. До RC подачі analyzer вже записує, GPIO3 verified LOW, firmware initialized/armed (це RX, не дозвіл насоса). Перевірити ready стан, потім задати reader condition active/slow/absent. Перший фізичний rising після цієї відомої межі має відповідати trace seq1. Якщо startup/masking/gap зруйнував відповідність — вікно fail/inconclusive, повторити окремим операторським restart, не best-fit.

Collector зберігає128 consecutive candidates без USB на імпульс; unknown/invalid не видаляти. Після128 owner freeze/stop, outputLOW. Лише тоді відновити reader та записати повний `rc03-trace H` +128 `R`+`E` cycle. Archive повторюється незмінно у звичайному темпі≤10frames/с, one-in-flight, без росту пам’яті. Capture_active у header0. Partial/mixed cycles не evidence. End window = останній pulse end, dump/restart до нього не входить. Одне вікно на boot; наступне — окремий operator restart.

Зберегти raw USB, vendor analyzer export і SHA-256, опис ручної нормалізації до `pulse_start_ns,pulse_end_ns` (цілі ns), без зміни фізичних значень. CSV має рівно відповідні128 pulses від відомого першого rising. Команда offline, пристрій не відкриває:

```sh
python3 scripts/verify/rc_capture_trace.py --trace frozen-export.txt --analyzer normalized-edges.csv --u-width-us <обґрунтоване-U-width> --u-interval-us <обґрунтоване-U-interval> --report comparison.json
```

Exit0=pass,1=fail,2=blocked. Зберегти кожне E_width=abs(RMTwidth−LAwidth) та E_interval=abs(GPIO rising interval−LA rising interval). Для кожного: E_width≤5мкс, E_interval≤10мкс; U≤1мкс окремо за D54-B, без E+U запасу. Не усереднювати, не округляти E, не видаляти outliers і не коригувати offset/drift. Absolute clock sync не потрібна.

Повторити вікна по доступному HJ width sweep, біля valid країв та для active/slow/absent reader під384B/100мс diagnostic load. Шість nominal map points і точні900/2100/18/22мс boundaries покрито host; записати фактично досяжні HJ значення, не приписувати йому довільні stimuli. D53 не потребує окремого генератора складних malformed streams.

З header max_rearm/max_processing/max_fault_lateness та snapshot ft зберегти очікуване/виміряне: rearm≤1000мкс від estimated falling+5000; completion callback→кінець core processing≤10000мкс. Це software instrumentation; оцінити її похибку/вплив, сам compile pass не є latency measurement. За потреби додаткового marker GPIO спочатку окремо затвердити профіль, не вигадувати pin.

Окремими електрично перевіреними fixtures виконати startup HIGH/LOW, missing pulses і сталий HIGH/LOW. Threshold100000мкс від observation start/останньої зміни; sticky/core fault до110000мкс, не час друку. Записати threshold_at/reported_at/lateness, конкретний reason, latch після resync/нових valid, output0. Fault/diagnostic counters не скидати для приховування помилки. Operational cutoff05 тут не приймається.

## P2: окремі600с loss/timing runs

Використати diagnostics-stress, trace off. Три окремі стабільні600с вікна50Гц: active reader, slow reader, absent reader. Bench робить слоти≥100000мкс, record384B із актуальними полями. За backpressure фактична передача нижча: записати attempts, bytes, busy_slots, write_drops, overwritten_snapshots до/після, max timings і fault flags. Не стверджувати3840B/s переданими за відсутнього reader.

Для однакових відомих rising boundaries у analyzer та diagnostic counter deltas визначити фактичну кількість correct pulses; accepted valid має точно відповідати, losses/duplicates0. Не припускати рівно30000. Startup unknown та післявіконні pulses не віднімати приблизно; зберегти raw capture і довести межі. Якщо через snapshot rate не можна однозначно співвіднести межі — inconclusive, повторити з контрольованим початком/кінцем сигналу та збереженими stable counter plateaus; не підганяти count. Втрати у rearm gaps рахуються. Інтервал потоку600с — за analyzer.

У цьому режимі немає всіх per-pulse RAM records; він підтверджує counts/loss/timing maxima, не accuracy всіх600с. P1 та P2 потрібні окремо. Весь час перевіряти GPIO0 LOW/permissionfalse.

## P3: reconnect/fault і завершення

За активного RC/max diagnostics зупинити/сповільнити reader, відновити, перевірити: максимум один раніше committed record, потім актуальний snapshot у тому самому темпі; немає catch-up history/burst. Реальний partial/zero write (якщо відтворюється) має дати counter/drop, без повторного remainder у production. Під stuck/fault текст може з’явитися пізніше, але in-memory/core deadline та GPIO0 незалежні від USB. Після resync latch не очищується.

Для кожного case записати expected/measured, source settings, reader mode, raw files/hashes, error/count/latency maxima, outcome pass/fail/blocked. Непроведене лишити not_run. Провал допуску блокує прийняття до виправлення чи явного рішення власника; tolerance не змінювати заднім числом. Майбутній конкретний receiver проходить власний gate до використання за D48; другий канал —07.
