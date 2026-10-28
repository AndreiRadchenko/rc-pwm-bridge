# SFDP спроба — зупинено до з'єднання

Дата 2026-10-28; користувач підтвердив, що RC, MD20A і насос від'єднані, а плата живиться тільки USB (D27). До звернення pyserial перерахував `/dev/cu.usbmodem1101`, USB VID:PID `303A:1001`, серійний номер `18:8B:0E:93:03:2C`; це enumeration, не ROM identity.

Команда: `/private/tmp/scopa01-replay-clean/bin/python -m esptool --chip esp32c3 --port /dev/cu.usbmodem1101 --baud 115200 --no-stub --before default_reset --after no_reset chip_id`.

1. У sandbox: exit 2; відкриття порту не вдалося, `Operation not permitted`.
2. Після дозволу sandbox escalation: exit 2; відкриття порту не вдалося, `No such file or directory`. Порт, що був присутній під час enumeration, до моменту повтору був недоступний.

Обидві спроби завершились до відкриття послідовного порту; ROM identity, SFDP, SPI-команди і запланований reset не виконано. Причину зникнення порту не встановлено. Згідно з процедурою, на помилці зупинено; T008 і фізичні gates залишаються відкритими. Цей запис не є доказом flash mode/frequency.

Повторне read-only перерахування після помилки показало лише Bluetooth-Incoming-Port і debug-console; USB JTAG/serial debug unit не виявлено. Повторного відкриття порту не виконували.
