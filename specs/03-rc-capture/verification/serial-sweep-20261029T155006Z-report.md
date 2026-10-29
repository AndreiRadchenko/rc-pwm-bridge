# Моніторинг HJ sweep800–2200мкс —60с

2026-10-29,15:50:06–15:51:06UTC. Користувач прямо дозволив підключитися й моніторити, повідомив новий поточний режим: HJ циклічно змінює800–2200мкс. Це поточне уточнення режиму, а не доказ режиму всіх попередніх1120invalid.

## Доступ та збереження

Порт /dev/cu.usbmodem1101 знайдено, lsof не виявив іншого відкритого reader. Стандартне pyserial відкриття O_RDWR середовище відхилило з Operation not permitted до отримання bytes. Помилку збережено у serial-sweep-20261029T154852Z.json. Після цього операцію звужено до OS O_RDONLY|O_NONBLOCK|O_NOCTTY — успішно. Налаштування termios, DTR/RTS не змінювались; application TX bytes0. Жодного flash/reset/команд керування навантаженням не виконано. Port закрито у finally після60,018340542с.

- Raw: [serial-sweep-readonly-20261029T155006Z.txt](serial-sweep-readonly-20261029T155006Z.txt),108385B.
- SHA-256: a10ea7ff0505259729b4a7fcc6e8e8a01fbcceb1c2e6f176b81634a9a7bbad84.
- Metadata: [serial-sweep-readonly-20261029T155006Z.json](serial-sweep-readonly-20261029T155006Z.json).
- Offline analysis: [serial-sweep-readonly-20261029T155006Z-analysis.json](serial-sweep-readonly-20261029T155006Z-analysis.json).

601 complete rc03 records, інших/boot/malformed lines немає. У всіх sid=f1d/profile fingerprintf1da070bd29de94e; це не firmware hash. Bin hash на платі не зчитувався.

## Старий committed record і поточний інтервал

Перший запис містить latest rising26,702678с; другий —180,214308с, gap153,511630с. Це узгоджується з одним раніше committed USB record, після якого йде current stream. Перший record збережено у raw, не приховано/не видалено. Дельти counters між першим і останнім включають час до підключення reader, тому їх не видаємо за нові60с.

Для600 records від другого до останнього latest-rising span59,920646с:

| Counter | Приріст між endpoints поточного потоку |
|---|---:|
| candidates |2996|
| valid |2396|
| invalid |600|
| discarded |0|
| capture fault events |0|
| duplicate edges |0|
| diagnostic overwritten snapshots |68095|
| busy slots |0|
| write drops |0|
| diagnostic slots |599|
| bytes accepted by driver |108031|

Це counters firmware між snapshot endpoints, не reference analyzer pulse count. Приріст valid+invalid=candidates. Великий overwrite означає заміну latest snapshots, не втрати RC. Cumulative busy1534 у першому current record накопичився раніше; у current tail він не росте. Наявність одного старого committed record відповідає дозволеній межі D43, але сам цей запис не є повним P3 reconnect acceptance.

## Ширини, status і карта

Усі600 current snapshots мають period19991–20008мкс, f0,o0,a0,sat0.

| Група спостережених latest snapshots | Кількість | Width,мкс | Status / calculated duty |
|---|---:|---|---|
| Нижче valid вікна |64|802–897|InvalidWidth(v3),утримане calculated0%|
| Усередині valid вікна |479|901–2097|Valid(v1),integer map0–100%|
| Вище valid вікна |57|2101–2203|InvalidWidth(v3),утримане calculated100%|

Суперечностей status проти погодженого width900–2100/period18000–22000 не знайдено в601 records. Для всіх valid snapshots calculated duty відповідає незалежно обчисленій погодженій карті/rounding; mismatches0. На invalid потік не перетворюється на нову accepted команду; на цьому плавному sweep утримані значення на краях0/100 узгоджуються з попередніми valid. Цей текст не замінює окремий host invalid-to-high regression test із різким переходом.

Cumulative software max timings наприкінці: rearm gap16мкс після estimated falling+5000,processing186мкс, fault lateness0 за відсутніх fault. Це RAM instrumentation, не зовнішнє вимірювання фізичних deadlines. GPIO0 напруга не вимірювалась — o0/a0 лише software fields.

Приклад lower invalid:

```text
w=889 p=20003 v=3 d=0 o=0 a=0 f=0
```

Приклад upper invalid:

```text
w=2101 p=20004 v=3 d=100 o=0 a=0 f=0
```

## Висновок і межі

Поточний sweep справді дає очікувані invalid width за межами900–2100. Валідація не потребує розширення до800–2200: це підтримуваний тестером діапазон, а не погоджена valid область мосту. Відмова sample не є capture fault сама по собі, тому v3 та f0 сумісні. Наявність valid ширин901–999 із duty0 та2000–2097 із duty100 також відповідає validate-before-clamp.

Монітор бачить лише latest≤10 records/с, не кожен≈50Гц pulse. Перевірено normal diagnostic reader, не bench384B/max-load та не600с. Analyzer/U/фактичні рівні conditioning, точність5/10мкс, zero-loss matching і фізичні GPIO0 waveform не перевірені; firmware source settings на платі незалежно не зчитані. T043–T047/T051 не закриваються цим partial observation.
