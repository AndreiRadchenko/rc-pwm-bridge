# Операторська процедура04

Стан: not_run; жодного апаратного дозволу поточним Implement немає.

Нормативна процедура — [quickstart](../quickstart.md#послідовність-фізичного-прийняття-без-насоса) P1–P5 та [контракт](../contracts/pump-output.md). До підключень потрібні Q02 H02/H03, Q03 conditioning, придатна методика/U й ідентифікований MD20A/Q06. Насос фізично від’єднаний для всіх boot. Окремі disconnected-driver і powered-driver записи; DIR–GND лише апаратно, напрямокмотора→06.

До run фіксуються firmware/config/source hashes, unit/profile/driver identity, power/ground/bias, прилад/calibration/timebase/threshold/sampling/storage/U, raw provenance. Не змінювати M04 допуски після вимірювань. Startup/reset/bootloader/upload≥5повторів кожногоcase/condition; pretrigger≥10мс; bootloaderLOW≥10с. Waveform65с/no-repeat до75с. StuckHIGH/LOW на кожному nonzero plateau: directGPIO3→GPIO0 terminalLOW≤112мс і10сguard. MCUAPI бюджети1/2мс перевіряються окремо, без cross-domainoffset.

Stress: output605с, evidence [t0+5с,t0+606с), max384B/100мс, active/slow/absent reader — окреміruns; RAMsummary/export післяfreeze. Фізичнийlower-bound міжfirst/last matched falling мінусU≥600с. Accuracy128candidates після+1с trace-step, LOW передstop, Ewidth5мкс/Einterval10мкс/U≤1мкс окремо. Фізичні GPIO0/driver-input frequency/duty/endpoints/voltage та повнийstress waveform — окремі raw records.

Результати T044–T050 залишаються відкритими до явного апаратного завдання та виконання. LOW MD20A — brake, не power isolation чи доведений нульвитрати.
