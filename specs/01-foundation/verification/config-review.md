# Огляд конфігурації target (T022)

2026-10-28. Команда `python3 scripts/verify/config.py --build-dir build/c3-01-final` — exit 0. Фактичні sdkconfig, graph, compile commands та map і hashes — `build-c3-01-final/`. Автоматично: target C3, DIO 40 МГц/4 МБ, single factory, application і bootloader log levels 0, tick 100 Гц, native USB/secondary none; build graph і map без esp_wifi/bt/esp_netif/lwip та заданих init symbols; останній -std для core — `-std=c++11`, PRIVATE `-fno-rtti/-fno-exceptions`.

Ручний source review: `app_main` створює supervisor і опитує його раз на 10 мс; валідність часу перевіряється до cast, далі profile, preload LOW, output mode, локальний setup. На помилці output решта setup не виконується. GPIO1/DIR, RC capture, LEDC, arming, network і app auto-reset не ініціалізуються. Діагностика передається окремою задачею через mailbox; USB write timeout 0, повтору після drop немає. Номінальний poll не доводить target lateness.

Generated config: bootloader WDT enabled 9000 мс; interrupt WDT enabled 300 мс; task WDT initialized 5 с; panic print/reboot enabled. Це SDK шляхи, не application retry reset; їхні bootloader/upload ефекти лишаються власникам 04/06, brownout — 06, operational watchdog — 05/06. ROM/bootloader/reset можуть впливати на GPIO до app, тому зовнішній bias і H03/SC-006 не доведені цим оглядом. Не перевірено фізичний startup або навантаження.
