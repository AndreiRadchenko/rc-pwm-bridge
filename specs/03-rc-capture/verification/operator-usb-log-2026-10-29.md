# Повідомлення оператора: USB rc03 працює

2026-10-29. Джерело — текст монітора, наданий користувачем у чаті після попередньої інструкції на ручний flash. Агент serial port не відкривав і плату не прошивав. Фактичний bin SHA-256 цього запуску не надано; `id=f1da070bd29de94e` є profile fingerprint, не firmware hash. Що саме підключено до GPIO3, на момент запису ще не уточнено.

Команда оператора:

```sh
pio device monitor --port /dev/cu.usbmodem1101 --baud 115200 --dtr 0 --rts 0
```

Нижче лише витяг із наданого повідомлення, не повний raw archive:

```text
rst:0x15 (USB_UART_CHIP_RESET),boot:0xf (SPI_FAST_FLASH_BOOT)
Saved PC:0x40382c5a
entry 0x403cbf10
rc03 q=2 id=f1da070bd29de94e sid=0 w=0 s=0 e=0 p=0 v=0 c=0 d=0 o=0 a=0 f=0 n=0,0,0,0,0,0 k=1,0,0,1,0 m=0,0,0 ft=0,0 b=1,1,0,0 sat=0
rc03 q=6e id=f1da070bd29de94e sid=f1e w=0 s=0 e=0 p=0 v=5 c=1 d=0 o=0 a=0 f=20 n=0,0,0,0,1,0 k=6c,0,0,2,85 m=0,0,a1 ft=195be,1965f b=3,1,0,1 sat=0
rc03 q=b5e id=f1da070bd29de94e sid=f1e w=0 s=0 e=0 p=0 v=5 c=1 d=0 o=0 a=0 f=20 n=0,0,0,0,1,0 k=b41,0,0,1e,1113 m=0,0,a1 ft=195be,1965f b=3,1,0,1 sat=0
```

## Розшифрування за поточним форматом

- USB application diagnostics присутня; це додаткове спостереження оператора після виправлення USB RX. Це не самостійна перевірка повного capture/transport acceptance.
- `v=5` Incomplete, `c=1` WaitingLow. Вхід програмно спостерігається HIGH; завершеного pulse немає. `w=0,p=0,n candidates/valid/invalid=0` не дають виміряної ширини/періоду.
- `f=20` hex =0x20/32 =StuckHigh, не число20мс. Накопичено один fault event; наступні records не збільшують count для того самого епізоду.
- `sid=f1e`=3870мкс початок спостереження; `threshold_at=195be`=103870мкс; `reported_at=1965f`=104031мкс. Fault у software clock через100161мкс від observation start,161мкс після100000мкс threshold (`m` lateness a1). Це один software instrumentation результат; зовнішня точність/час фізичного рівня аналізатором не підтверджені, повного stuck acceptance немає.
- `b=3,1,0,1`: ReadyDisabled, одна startup спроба, startup errorNone, apply_low API success. `o=0,a=0` — нульова команда/permissionfalse у firmware, не виміряна GPIO0 waveform.
- `k` overwritten зростає через latest-only snapshots; busy_slots/write_drops=0 у витягу. Overwritten snapshots не дорівнюють втратам RC pulses.
- `m` rearm/processing0: завершених вимірювань не було, тому ці бюджети тут не перевірені.

GPIO3 з вимкненими внутрішніми підтяжками може бути невизначеним без зовнішнього bias. Сам цей log не встановлює причину HIGH і не доводить ні наявності, ні відсутності pull-up. Якщо джерело імпульсів підключене, потрібна перевірка фактичного входу/ground/conditioning; залежна діагностика очікує уточнення користувача. Жодних автоматичних GPIO/апаратних змін за цим log не виконано.

T043–T047/T051 залишаються відкритими. H02/H03/Q03, waveform/accuracy/loss600с/reader stress фізично не прийняті цим витягом.
