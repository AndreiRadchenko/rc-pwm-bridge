# Підготовка target environment — не завершено

2026-10-28; T013 в роботі, E01/Q01 відкриті. Target build/config review не виконано.

- ESP-IDF v5.5.6 отримано у `/private/tmp/scopa01-esp-idf-shallow`; `git rev-parse HEAD` = `880a203ae603d08a94d06aa01e0d6761958e5e9e`. `git submodule update --init --recursive --depth 1` завершився exit 0. Фактичний `git submodule status --recursive` збережено у `/private/tmp/scopa01-submodules.txt`, SHA-256 `cc4323992e5421340b2dba85762d3bb11592a0e2fd49bb7518196625096e1b59` (тимчасовий файл, не portable evidence).
- `tools.json` із pinned commit: `/private/tmp/scopa01-tools.json`, SHA-256 `799bf92db9d1f1d584ed8afe8d98ca4bda49dad3796fdd0516b4c8d62e818a09`. Офіційні constraints v5.5: `/private/tmp/scopa01-constraints.txt`, SHA-256 `f4227d411b143e0bdab0b966f182bfcb47f3a999287c49a5586827c6b3430d05`.
- Архів riscv32-esp-elf `esp-14.2.0_20260121` завантажено, SHA-256 `caa7103e014c61939eea4656cd4effdc34930da0b76cd59ad1adb7bfaeef923c` збігається з manifest; інсталяція та версія executable ще не перевірені.
- `env IDF_TOOLS_PATH=/private/tmp/scopa01-idf-tools IDF_PATH=/private/tmp/scopa01-esp-idf-shallow python3 /private/tmp/scopa01-esp-idf-shallow/tools/idf_tools.py --non-interactive install --targets esp32c3` завершився помилкою: Python TLS `CERTIFICATE_VERIFY_FAILED` при завантаженні `riscv32-esp-elf-gdb`; retries вичерпано. Перевірку TLS не вимикали. До продовження усунути причину trust store та повторити install, перевірити actual versions/hashes усіх необхідних tools.
- Full Python closure, hash lock і replay не виконано. Цей запис не є evidence target build, runtime або фізичного прийняття.

## Виправлення Python CA (2026-10-28)

Переносимий Python 3.11.7 мав OpenSSL default CA path на машині збірки Python, якого тут немає (`ssl.get_default_verify_paths()` повертав `cafile=None`). Створено `tools/environment/python-ca.sh`: у поточному shell він встановлює `SSL_CERT_FILE=/etc/ssl/cert.pem`, не вимикаючи перевірку сертифікатів і не змінюючи системний Python чи глобальну конфігурацію. Системний bundle SHA-256 на момент перевірки: `9dae8d76e55cb08991f2b672d58999ea15560d910759c16b544f843bdffbb994`; стан bundle може змінюватися з оновленням ОС.

Перевірка у shell після `source tools/environment/python-ca.sh`: Python повідомив `/etc/ssl/cert.pem`, `ssl.create_default_context()` завантажив 128 CA. Окремий `urllib.request.urlopen` із тим самим `SSL_CERT_FILE` відкрив саме URL архіву `riscv32-esp-elf-gdb` із попередньої помилки та отримав HTTP 200; TLS verification залишалася увімкненою. Саме встановлення IDF tools повторно не запускалося; T013/E01 залишаються відкритими. Для подальших команд в новому shell спершу виконати `source tools/environment/python-ca.sh`.

## Повтор T013

Після виправлення CA `idf_tools.py --non-interactive install --targets esp32c3` завершився exit 0. Фактичні версії GCC `esp-14.2.0_20260121`, GDB `17.1_20260402`, OpenOCD `v0.12.0-esp32-20260831`; усі шість архівів (включно з CMake/Ninja та ROM ELF) перевірено за SHA-256 із pinned `tools.json`. SDK requirements, constraints, tools manifest, submodule inventory та hashes записано до `tools/environment/`. Target Python closure/replay і build досі не виконано.

## Python closure staging (T014)

62 distributions завантажено і зафіксовано у `requirements.lock.txt` і `wheels.sha256`. Готового wheel для esptool 4.12.0 не було; wheel зібрано з `esptool-4.12.0.tar.gz` у staging із setuptools 84.2.1, wheel 0.48.0, packaging 26.3, `--no-index --no-build-isolation --no-deps`; SHA-256 вихідного архіву та wheel зафіксовані в manifest. Staging `pip install --no-index --require-hashes` і `pip check` завершилися exit 0. Повторне незалежне відтворення T016 ще не виконано.

## Офлайн replay (T016)

`/Users/mac/.platformio/penv/bin/python3 -m venv /private/tmp/scopa01-replay-clean`; `/private/tmp/scopa01-replay-clean/bin/python -m pip install --no-index --find-links tools/environment/wheelhouse --require-hashes --force-reinstall -r tools/environment/requirements.lock.txt` — exit 0; `python -m pip check` — exit 0, no broken requirements; `idf_tools.py check-python-dependencies` із IDF_PATH, IDF_TOOLS_PATH та IDF_PYTHON_ENV_PATH — exit 0, requirements satisfied. `python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json` — exit 0. Hash lock, wheel hashes, SDK manifest і inventory у `tools/environment/environment.lock.json`; replay evidence — `tools/environment/replay-evidence.json`. Перший replay з `--ignore-installed` мав дубльований pip metadata і не використаний як evidence; наступний clean replay з `--force-reinstall` усунув дубль, checker перевіряє відсутність дублікатів. E01/Q01 закриті для цього environment; target build not_run.

## Чиста target-збірка (T020/T021)

2026-10-28. `python3 -m unittest discover -s tests/tooling -v` — exit 0, 7 tests; fake subprocess/profile fixtures перевіряють CLI codes, зайнятий каталог, invalid profile до configure, відсутність flash/reset/monitor команд та missing/forbidden evidence.

`python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-01-final` — exit 0; чистий каталог, 511 build steps; app image `0x24950`, factory partition `0x100000`. Це фінальний source snapshot після проміжного `build/c3-01-clean`; останній збережено, але він застарів після правки bootloader logs і flash identity check. SDK v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, submodules та lock перевірено environment checker; C3 toolchain esp-14.2.0_20260121. SDK скопійовано без змін у `build/scopa01-idf-sdk` для обходу проблеми нормалізації symlink `/tmp` у CMake; Git identity/submodules перевірено повторно.

Artifacts, config/map/compile commands і SHA-256 джерел збережено у `build-c3-01-final/`; binaries залишаються в `build/c3-01-final/`. Збірка не відкривала порт, не прошивала плату і не є physical acceptance.
