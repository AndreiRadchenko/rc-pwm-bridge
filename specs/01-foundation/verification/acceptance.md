# Початковий індекс доказів 01-foundation

Дата: 2026-10-28. T001: до змін реалізації; SHA-256 — знімок наявних відстежуваних і невідстежуваних файлів. Сам цей запис не входить до власного знімка. Подальші зміни потребують нового fingerprint.

Git HEAD: `8638d72a259762708094eb2430a3c8308e7e34fa`

```text
## 01-foundation
?? .agents/
?? .gitignore
?? .specify/
?? AGENTS.md
?? PROJECT_FOUNDATION.md
?? README.md
?? docs/architecture.md
?? docs/requirements.md
?? docs/roadmap.md
?? firmware/
?? hardware/
?? scripts/
?? speckit-iteration/
?? specs/01-foundation/checklists/requirements.md
?? specs/01-foundation/verification/
?? specs/README.md
?? tests/
```

| Gate | Початковий стан |
|---|---|
| E01 | open: closure/replay target не підтверджено |
| H01 | design визначено D18–D20; profile preflight не виконано |
| H02 | blocked: фізичні живлення/bias не перевірені |
| H03 | blocked: числова методика та waveform не підтверджені |
| H04 | pending: RC, власник 03 |
| H05 | pending: MD20A/силовий тракт, власники 04/06 |

| Критерій | Початковий стан |
|---|---|
| SC-001 | not accepted; перевірку не завершено |
| SC-002 | not accepted; перевірку не завершено |
| SC-003 | not accepted; перевірку не завершено |
| SC-004 | not accepted; перевірку не завершено |
| SC-005 | not accepted; перевірку не завершено |
| SC-006 | not accepted; перевірку не завершено |
| SC-007 | not accepted; перевірку не завершено |
| SC-008 | not accepted; перевірку не завершено |

ROM/USB доказ у board-review підтверджує лише частину ідентичності; не є host, build чи фізичним startup acceptance. Конфлікту з наявним acceptance.md немає.

## SHA-256 вихідного стану

```text
1e259572c73b76e9df5bca8b00d798a20da0f97bd8f53688f54d550f6caa16dd  .agents/skills/speckit-analyze-guided/SKILL.md
2e89ffed73764eb9d3ee2d9fba7e245a5e73229b809ddec52409ca74c44e585d  .agents/skills/speckit-analyze/SKILL.md
9a7fecc14b2d8a0809b6e27fb8e2aca47e4554d9649ac997637501e4d0ac29ce  .agents/skills/speckit-checklist-guided/SKILL.md
4eb7e030d04014e6bbf5177f84aeef19b11adfa8470a14dd18506f65c7cf9dcb  .agents/skills/speckit-checklist/SKILL.md
5095627c89a249d402ad2d0fad425ea0c0bd5e8559e9d6f5b682dea4f513ed54  .agents/skills/speckit-clarify/SKILL.md
c57bf65daa9c23e24c97071bc0e4f930b3f458549b52792c4a65830de6b28f6b  .agents/skills/speckit-constitution/SKILL.md
1355b748919c7750cc87be94534192e5ff227229c14cbd34ca993a5f977a4ce2  .agents/skills/speckit-converge/SKILL.md
248c2fa4be8a969ea771c88c38546d6735d7d4994bbe698bbabd18f53f88f0cd  .agents/skills/speckit-implement/SKILL.md
c5522489fd177028e02ff4f1588a06bde811a0146cdc1d3495812694d6ea41b3  .agents/skills/speckit-plan/SKILL.md
6313b902b49bf2ae3582421be21c2453ff98a863974acca3e6a83cdad9d4b70b  .agents/skills/speckit-specify/SKILL.md
2dceed4345af65dbc6625157be8817dc35f2b0943b5896815ef1bc4d3421efba  .agents/skills/speckit-tasks/SKILL.md
2c00fdc774cb9b435ea8566c7efa8b208568d533d57529e61c35f18aa52d6c67  .agents/skills/speckit-taskstoissues/SKILL.md
a49e12a043c94d52b72543202f4a9154bc9b7a448a350066215e11b0bc23a90b  .gitignore
74b6cf1027539037c9e000c2b46d46269a649597365200b8db6527715921537a  .specify/init-options.json
09e7128ed5a92a33c74e82ea3935a8d72ca80d8649886885460f0be8e8d3f85f  .specify/integration.json
c43ad9c187cadcdefef1ace6786073d22f5232dbcd3239acfd822170a46cfb56  .specify/integrations/codex.manifest.json
38f09860f41a948d306728a7c87279b581327e7f65d5523dee063a46da20b23d  .specify/integrations/speckit.manifest.json
8fc4b39294bae0c10d59547ad7d415b6f0e890d656cbf7171dd567f4e9054711  .specify/memory/.constitution-template.json
8b25fb7dabe3abac8c83194a1ce8f6ab649049ebd7ac51b2a43b8792d02596a3  .specify/memory/constitution.md
a7d8a14ecf87332b600cd966b5d0e7cb9d594abce7e4d1ee4372b2b5b3efff06  .specify/scripts/bash/check-prerequisites.sh
1aec64a0a5b9d9696b305cdef7051c7c37208b04083f3c34bf04c63eead60258  .specify/scripts/bash/common.sh
7c60a87b4a43947befd1c58cbab6baf27eca850e8b5969281fb7ecf4a7e64bd8  .specify/scripts/bash/create-new-feature.sh
4469b22960f43c07c33dca00de6dedb252145e9a9ce8fbb0e63be82e02b082ab  .specify/scripts/bash/setup-plan.sh
cf21ba2212b4dd5b435c5ea8527500cfd27768b86c0bbc7ebc3207759f118d27  .specify/scripts/bash/setup-tasks.sh
709d8ab8384a3a49f5e0f64479f71553ef6d6f8bb4f00281b05f47837993b536  .specify/templates/checklist-template.md
ce7549540fa45543cca797a150201d868e64495fdff39dc38246fb17bd4024b3  .specify/templates/constitution-template.md
1801f0a51794304577ef2a812ff90eaa73e210e73ef74fb645baeb48cde97782  .specify/templates/plan-template.md
023556fa99c7179e9fecb928fff8e0ac218fa72d9072810cb3892397eac04598  .specify/templates/spec-template.md
12857a471032e2d91a23f79b4e3fe047269e5f4eb0d066aba80507e9eee6809c  .specify/templates/tasks-template.md
a3b7c17724df02c7f2ad4cfc58435dd428f11eaa13bd07fa5e46f7726142b2b7  .specify/workflows/speckit/workflow.yml
c10784f89532a7008c882d9d2dfe064f595c1ecf10290de8f46cd5b7140f6814  .specify/workflows/workflow-registry.json
9391cffdf5f97070e57e711a95d07ee43e13d4e9425092d96b038fff3cb8b383  AGENTS.md
925acf1cc802827a8c7edd3c2b1ad580b0b8e513fd8056315b948033a58c80a9  PROJECT_FOUNDATION.md
5de3288cf4018df4280da43e3618da04b2ca392e08829ce6d646db14a39a13c5  README.md
64c2130cec480167d06d7a149e3236df2aa46a39bda13d5cc45b38c873d5bd1c  docs/architecture.md
179b1c043f67bb35efb984ad348a3e918d653cbe27b809603948df5384fb9c86  docs/requirements.md
1dd24e26b850b6d226a850d945130f8376f8e33dc4d0060791722789cca64eae  docs/roadmap.md
092cf19c22fc10559c4a2e556bf39cefa4ab1ff6d3a02c186057e86d70b22c39  firmware/README.md
ae4bdb93347fd5f7cd457a9f8d1a17b1bbaf900dc544b2abca89b96b59f059f7  hardware/Cytron_MD20A_datasheet_rev1.0.pdf
7413aab09aa1022bf4a569d573c5078400f4ce7a4962587fc39d0560dddfad7f  hardware/README.md
c0c770ef639612bcfeca7ed7472c4d0b190032b43332c84b52fda81f3133a07c  hardware/esp32-c3-pinout.jpg
52ccc70239f240aef1679c6a02657606c4a035d51fc87b21aa77009014231aec  hardware/esp32-c3-supermini-profile.md
26b4d940248a02ef4a3e99f29afc133867d9c4b0cfb8db0147de2f2f7cd01cee  hardware/esp32-c3-supermini-review.md
833fc000b4b3c3d39c496fcbd597fed5806956503ce7390b19cc8ae82f19f968  hardware/esp32-c3_datasheet_en.pdf
36ab8c15ab2626a4c55ddf45ceb3a02d9be5e18dd465bc9602f31cc8927ee041  hardware/esp32-supermini.jpg
db8becb55296b519afb52c2b43ea6734d1386d9128556961f5e3b95e9168d1d9  scripts/README.md
40e9f829aa47d0a9d70ac2c9b83b5987846f50f379b08b6491207c483258587e  speckit-iteration/01-foundation/manual-step.md
3d1d0a4dcc244162e169fe90ae574adc30df4f39037112253cf900569d256410  speckit-iteration/01-foundation/plan.md
cd8beb208fc691194302ba5a3d7418f4b62721e4fadbea57b1f621ef35705459  speckit-iteration/01-foundation/specify.md
f00dcea0f73b29d1cc334f94ff538f0e592ccf6fecf7964ca0f7b6a70e22d640  speckit-iteration/02-control-core/manual-step.md
2aa0e5209267308ac8caf622371a4791b158f61d792ae56dae3d83e16deb5f3d  speckit-iteration/02-control-core/plan.md
00ec97de8d6c2604c3dfd89245ff62d6dbaa15f76d6a7d18bd191ceeee4f0359  speckit-iteration/02-control-core/specify.md
43f6b90c4a6730177b2d38aefdd7de83453895a2aaa230fdbbd720243aa7e057  speckit-iteration/03-rc-capture/manual-step.md
5be99ed7ccfcf0d6c9383deb3d41e1b5f633a935668a2f5e2282b0ed53ec96f3  speckit-iteration/03-rc-capture/plan.md
52e6e86b5578bb5717ddf959c2ee380127b122a520351bbe2d4f43e5a5a7e5e0  speckit-iteration/03-rc-capture/specify.md
3ca878a3476bb157fe2a9849b2e83960c7f0641002e37213023ed00bdbad0ca1  speckit-iteration/04-pump-output/manual-step.md
0809ab118d1d2c7246c81e8ccce821cf2c8ae71b55cc754c13829331e5918eeb  speckit-iteration/04-pump-output/plan.md
01fc5307ac7b6087939401fe072db5885c7d7902f76ca88996e265400a362e39  speckit-iteration/04-pump-output/specify.md
14e4231b159233d1de6e681d87ae74d120b539722ef845f09c13609f2938c814  speckit-iteration/05-failsafe-arming/manual-step.md
0ead268f0b5599bd67e0ae924e3c73f8320a417eb60d2ab09177ec3a98c5b2e5  speckit-iteration/05-failsafe-arming/plan.md
bcb6068900f795f7418790ccd1241e7b3f91b04d1d751beb4bd9a2e12241be0e  speckit-iteration/05-failsafe-arming/specify.md
6be33fdd56850c817f3876c6bde334096917c1480ae69b48756ce243564bf7b4  speckit-iteration/06-pump-validation/manual-step.md
b71795d12cd40b4dad32b5c45c81b1374b52ed60ff56059fd6a6f89be1555e65  speckit-iteration/06-pump-validation/plan.md
0ee259fe5ebb4217b8b4cceccb1294560a91b439d4de777cffca778dbfff2022  speckit-iteration/06-pump-validation/specify.md
d568bb5d117443da1444db9f34ddb84f67e8a8e1d4b93d23fd1a2e2b01c18092  speckit-iteration/07-ignition-channel/manual-step.md
905a4393114c293a6ea02fb5b5156935ec35c9543f2351b287a94bf7f8419601  speckit-iteration/07-ignition-channel/plan.md
7d87b047e9c843f8e91cb9b701a8555e336dbccadac1363bc39b7a61068a24d0  speckit-iteration/07-ignition-channel/specify.md
9adb63af35cd46b4614587b719266f14ea09896d49f606833a1a6c0c9e564ebf  speckit-iteration/08-esp32-portability/manual-step.md
6c3cb9b7a2a93ab183fd2d5f99ef07ae8e6e92710cd39917756bbc38c1bfe59c  speckit-iteration/08-esp32-portability/plan.md
07959c9f6ac18c2d921cc0e82f8437cea278f77bf781fac2ac960395e10115df  speckit-iteration/08-esp32-portability/specify.md
f869e03beccf0a807ce2d80e345f96f73ecdaf721d124b46eea8cfb55a2c982b  speckit-iteration/09-arduino-nano/manual-step.md
e2fd9e7ab2bfb5aa443462bf70cb08a6c0f87534be82c8454282e63f25980c7e  speckit-iteration/09-arduino-nano/plan.md
db3c8331c40d2823a9e4b337beecd40cb31d4cda487a289d732f2e52cf820aee  speckit-iteration/09-arduino-nano/specify.md
d0c8b4bdd5015d8ef87ae997ac8b9892d373c3326e5cc68f08d904729d248b07  speckit-iteration/README.md
a6a6e66f0710ebf0a13691a6c20023eb035cda20265654a63840014c9383029e  speckit-iteration/constitution-prompt.md
755bbc089ba96f9d8024b7c73c97477369ad32209eb11761a92887db91ecd7bd  specs/01-foundation/checklists/requirements.md
f2ad1095d634d478219c217d66c1e08fc694e6cfee44fc284582491831dab887  specs/01-foundation/verification/board-review.md
4341e6aad6e2fbd82c525609c6317b5ade6029b8a51ca14abc537467c774b26b  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-capture.json
f4a28108077a09551472240457fed934ed9d6d233193f6764e2317e924e46698  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-console.bin
f4a28108077a09551472240457fed934ed9d6d233193f6764e2317e924e46698  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-console.txt
f47a8ec3e9aff2318d896942282ad4fe37d6391c82914f54a5da8a37de1300c6  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-header.bin
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-header.stderr.txt
d401ad94ecad70ce67833f6c1cb6f0deb97afb27d898dafcc5dd00b899051944  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/boot-header.stdout.txt
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/chip.stderr.txt
f717eaf89775b7c5c5b524afd008ab51c7469f56636bc6c90be9086bc2e86ac2  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/chip.stdout.txt
56803fcdbef0f52c9aef90bc46e5959a6f4960fd9aff7e42434a05aa30e2aa8b  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/flash-metadata.json
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/flash-status.stderr.txt
085b30361e56e747f43365ba3e5859bbb03afc5d39eed472176abae2f4e8c87c  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/flash-status.stdout.txt
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/flash.stderr.txt
8a27336e862f7a1a0b994b2eff6ac31c290569ed532d9c90d0ad1f627e737240  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/flash.stdout.txt
c8450f762c49b4feb619e87679f47d80057d78af0505e105763916ae02439b23  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/observations.json
f47a8ec3e9aff2318d896942282ad4fe37d6391c82914f54a5da8a37de1300c6  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/partition-table.bin
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/partition-table.stderr.txt
d558e12cc8dcaf78d81fd07d4d941df37de5ed0c56f9daa9222fccec21b9e28e  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/partition-table.stdout.txt
1c1c60504371670a2f80447935c6a5b6b763c4aedb244b3334a90752768b3220  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/rc-pwm-board1101-boot-read.py
39ef6938335d875ab368955b523b1c7fe162166b757294737121b8f9a6e35005  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/rc-pwm-board1101-inspect.py
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/restore-boot.stderr.txt
088df6fee82e84c6a49dd99a98d7aa7c3ae7371b525bdcbb34c8f84d83a359fb  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/restore-boot.stdout.txt
cdc7d1c420ec6453b81cfa8eb7deb880ae6d923c15ff1ff01e30659dcc0a2e52  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/rom-queries.json
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/security.stderr.txt
332bf43eb69f4f0ffcb66ab48aa7149394dad47cc7068a82852f2b06d095e655  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/security.stdout.txt
fee8e908c3d8221f74e71f80f22d29d0d6d4f4c7f5ca580241485775dcc974cb  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/sha256.json
c080ece0c44d92d4c1d9d52f0e4f7881f9cfd5fde54015ad2bbc1d72170c5a80  specs/01-foundation/verification/board-usbmodem1101-2026-10-28/usb-enumeration.json
69216ba1feb9ac2cc07c807ce5161577ef867d3d175770b6137febfb9b5d5a1b  specs/README.md
b295bcf5a402cc2f14e3d9e62ae09cae731e7b5c576828888012102930fe6195  tests/README.md
300718e05b1909c4d03db43e79a5a459f73c84aca4d8491beed8833f940b5854  docs/decisions.md
9fd1ae958fc0f9e9d533d34d7c1ff2fa9c283efb732edbed7ed5a9ec27481b47  docs/open-questions.md
1a65d91b651983a86587870021d32e1e7fb27a1ec8f2ebb17316a238d3734e0c  specs/01-foundation/analysis-resolution.md
31bb33e1e45fdd20f3226a551a9c0e876d418039c0f2925315812935dd5b9ca3  specs/01-foundation/checklists/foundation-readiness.md
45b9a9be224a910931be739c682865f83d630b60d442fc5ab7996c54a6f1d1fd  specs/01-foundation/contracts/board-profile.md
74c305de8e36cb9021bebe14a2cc6329eaadf646a1ec5bb2b19e4267a3e786c0  specs/01-foundation/contracts/build-and-evidence.md
55d6e7d7e273fbb228399dd74235c84f20889300a7751f9522d54cfaeff87797  specs/01-foundation/contracts/startup.md
e2f660f70739734626d688dccdcf173e3161228f70fc41cd295a6329075fa45b  specs/01-foundation/data-model.md
5b1b25531f8af27c2aab2340fdae66502633a1f6b3796635b61aad5d4a078ff0  specs/01-foundation/plan.md
3d534e842bc285f1540223164494022941c6e0a455c5ffdb4cdd51f911f98366  specs/01-foundation/quickstart.md
c591af3f7da582a4fd6234d95845a821876b20b52b67e62bd583e88e1d2e3d6e  specs/01-foundation/research.md
7df695618cfc87ff2c83e28b629716c6209f76cb642afdc97111d394fafe1c2a  specs/01-foundation/spec.md
680f7af56a6bf02e339b40248e0855dc94b833fb6b134834028aa14fe85cc058  specs/01-foundation/tasks.md
```

## Проміжний прогрес implementation

T001–T007 виконано (T005 перевірено на відмові конфігурації за відсутності джерел; пізніше host build зібрано). T023/T026 та частину host-тестів реалізовано; host CTest 2/2 пройдено, negative probe повернув 1. Це проміжний результат, не завершений T036 та не прийняття SC-003: перевірки ще доповнюються. T008 очікує первинного підтвердження конкретної flash; H02/H03 та фізичне прийняття не виконано.

T013 зупинено через Python TLS `CERTIFICATE_VERIFY_FAILED` під час install target tools; докладно — `build.md`. T008 очікує конкретної моделі flash; T011 — параметрів приладу та bias. Жодна фізична дія не виконана. Host-докази остаточного запуску — `host.md`; актуально 14 із 40 задач позначено виконаними, прийняття ітерації відсутнє.

## Продовження після відновлення CA

T013–T016 виконано, E01/Q01 закрито для зафіксованого host/SDK environment; `verification/build.md` містить подробиці. Host suite повторено з оновленим manifest, див. `host.md`. SDK leaf clock/diagnostics підготовлено, але T025/T027 залишено відкритими до відповідної перевірки/компіляції. T008 лишається відкритим: JEDEC ID відповідає декільком можливим моделям in-package flash; параметри mode/frequency не внесено без достатньої первинної підстави для конкретного екземпляра. T011 залежить від даних приладу та bias; фізичні дії не виконувалися. Ітерація не прийнята.

D24: користувач обрав документальний пошук T008; підтверджено загальну специфікацію in-package flash ESP32-C3, але точну модель і сумісність конкретного режиму з цим екземпляром не доведено. Результат — `research.md` RCH-08; T008 залишається відкритим.

D25: збережено суворий доказовий gate T008; підготовлено `sfdp-procedure.md`. Процедуру не виконано; доступ до плати потребує окремого явного завдання.

D28: T008 виконано як обґрунтований flash design після узгодженої ROM/SFDP діагностики; доказ — `sfdp-2026-10-28/`, розбір — `research.md` RCH-08. Фактичний boot не перевірено. Плата залишена у ROM download mode.

D29: прилад H03 відсутній; LED-стрічка не замінює числового вимірювання. H03/SC-006 залишаються blocked.

D30: користувач повідомив про можливе джерело 3,3 В із польотного контролера; його характеристики та H02 залишаються неперевіреними.

## Індекс перевірок T037

| Вид / задачі | R / FR / SC | Умови й очікування → фактично | Доказ |
|---|---|---|---|
| host T003–T005,T023,T026,T032–T036 | R02,R08,R12 / FR-003,007,008,010,011 / SC-003 | fake clock/ports, deadline/faults, без MCU → CTest 2/2, negative probe exit 1 | [host.md](host.md) |
| target environment/build T013–T021,T024–T028 | R01,R02,R10,R12 / FR-001–006,014–016 / SC-001,002 | offline lock replay і clean C3 build → exit 0; без serial/flash | [build.md](build.md), [source hashes](build-c3-01-final/source.sha256), [artifact hashes](build-c3-01-final/artifacts.sha256) |
| config/source review T019,T022 | R01,R08,R10,R12 / FR-007–010,013–015 / SC-005,007 | generated config/graph/map + ручний review → software review пройдено, hardware effects відкладено | [config-review.md](config-review.md) |
| board/profile T006–T010 | R01,R02,R08,R12 / FR-004–006,016 / SC-004 | design, SFDP, preflight tests → software pass; H02 фізично не виконано | [board-review.md](board-review.md), [SFDP](sfdp-2026-10-28/), [hardware.md](hardware.md) |
| physical T011,T012,T029–T031 | R08,R12 / FR-012–015 / SC-004,006,007 | H02 живлення/bias і H03 GPIO0 waveform → blocked/not_run; прилад недоступний | [hardware.md](hardware.md) |

SC-001/002/003/005 мають software evidence зазначеної області; SC-004 і SC-006 залишаються відкритими. SC-007 має записані неперевірені сценарії/власників, але не означає фізичної перевірки. SC-008 вимагає фінального review T039. RC і MD20A gates належать 03/04/06 за D21 і не є physical pass.

## Review конституції і простежуваності (T039)

1. **Bounded local pump control**: каркас локальний, permission=false; RC capture/LEDC/arming і додаткові канали відсутні.
2. **Explicit safety and fail-off responsibilities**: програмний LOW, зовнішній bias і незалежне аварійне відключення розрізнено; фізичні H02/H03 не пройдені, LOW MD20A — brake.
3. **Portable core and platform boundaries**: host suite та фактичні target flags перевірені окремо; апаратні calls у платформних leaf adapters.
4. **Reproducibility and traceability**: environment lock/replay, clean build, config/map та source/artifact hashes записані; середовище містить локальні шляхи, повторно перевіряється gate. Після build змінено лише `firmware/README.md` і `scripts/README.md`; зібрані джерела не змінювалися.
5. **Bounded iterations and authorized actions**: 01 не прийнято; SFDP виконано за прямим завданням, flash/керування навантаженням не виконувалися; операторські T012/T030/T031 відкриті.
6. **Honest decisions and scoped uncertainty**: живлення від польотного контролера неперевірене, прилад H03 відсутній; RC/MD20A gates відкладені за D21; bootloader/upload — 04/06, brownout — 06, watchdog — 05/06.

Повний зв'язок R/FR/SC→T збережено в `../tasks.md`; підсумкові T→evidence наведені в індексі вище. SC-004 і SC-006 без фізичних доказів відкриті. Цей review не є рішенням про прийняття ітерації.

D35: фізичне прийняття відкладено. T011/T012/T029/T030/T031/T040 залишаються відкритими; завершення 01 не заявляється.
