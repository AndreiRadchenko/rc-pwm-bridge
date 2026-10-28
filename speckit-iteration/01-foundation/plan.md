# Plan prompt — 01-foundation

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `01-foundation`. Requirement traceability: R01,R02,R08,R10,R12. Prerequisites: —. Questions to resolve or explicitly scope: Q01,Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/01-foundation` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/01-foundation/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Establish a reproducible native ESP-IDF repository foundation and identify the exact ESP32-C3 board and MD20A interface before assigning pins. Deliver a minimal safe startup skeleton, not operational pump control.

Use firmware as the ESP-IDF project root with main and components boundaries from architecture. Select the SDK version after verifying availability and C3 RMT/LEDC APIs, record dependency pins/lockfiles and commands. Resolve strapping, flash, USB/JTAG and UART pin conflicts from the actual board schematic. Define the external fail-off and emergency cutoff design separately from startup GPIO code. Never guess a board from the chip datasheet.

Preserve the accepted behavior: Document the exact board revision, flash, power path, RC input conditioning, RC/PWM pins, permanent MD20A DIR-to-GND jumper (no MCU DIR pin/control), and PWM boot/reset bias. Select and pin a stable ESP-IDF/toolchain and a portable C++ baseline suitable for a future constrained Nano adapter. Keep Wi-Fi and Bluetooth disabled. Provide real build and host-test entry points; do not present future platforms as buildable.

## Validation and deliverables

A clean C3 build on documented dependencies; meaningful host-test harness check; inspection of disabled radio configuration; board/pin review and no-load startup/reset observations when hardware is available. A build does not prove physical boot safety.

Produce `specs/01-foundation/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
