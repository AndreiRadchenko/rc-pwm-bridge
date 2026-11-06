# Pinned target builds04

2026-10-30. Фінальні clean builds **9/9 exit0**, кожний у новому `build/c3-04-reviewed-*`. Pinned environment check exit0; ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, native C3 profile `c3-supermini-pinout-01`. [Environment](environment.md), [точні команди/exit/logs](build-results-reviewed.json), [config/ELF review](config-review.md).

| Variant | Manifest fingerprint prefix (64bit runtime ID) | App bin bytes | Build/config |
|---|---|---:|---|
|normal|`b9b6cd345d1b4e05`|188928|pass / pass|
|waveform|`c98c93822f37c79d`|189152|pass / pass|
|stress|`91c78d886f66f87b`|189264|pass / pass|
|accuracy-0|`4d993faae17c65a4`|190304|pass / pass|
|accuracy-1|`51bbac57676ae138`|190304|pass / pass|
|accuracy-2|`3fc8a0fd6502bed6`|190304|pass / pass|
|accuracy-3|`b447e59a3d9b1f70`|190304|pass / pass|
|accuracy-4|`f5d203210e4ef62b`|190304|pass / pass|
|accuracy-5|`d5d156a05c095fb0`|190304|pass / pass|

Повні SHA256 manifest/sdkconfig/generated header/compile graph/ELF/map/application/bootloader/partition bins і поточних sources/tests/scripts/docs — [software-evidence.json](software-evidence.json). Кожний manifest містить source_hashes; вони повторно зіставлені з фактичними firmware files. Accuracy step0..5 має окремі manifests/configs/identities. Усі app bins містяться у single factory partition1MB.

Перший `build/c3-04-final-normal` failed на GCC misleading-indentation; failure log [build-normal.txt](build-normal.txt) збережено. Intermediate r2 builds pass, але вони передували/перетинали фінальний cursor review: не є фінальним evidence. Усі reviewed builds виконані після останньої firmware зміни.

Wrapper виконував тільки build. Стандартні рекомендації flash у stdout ESP-IDF не виконувались. Build/config/static ELF pass не доводять запуск MCU, стан GPIO, частоту/рівні, runtime high-water або фізичні gates. Порти/serial/monitor/reset/flash/load не використовувались.
