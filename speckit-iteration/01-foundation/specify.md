# Specify prompt — 01-foundation

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `01-foundation`. Requirement traceability: R01,R02,R08,R10,R12. Prerequisites: —. Questions to resolve or explicitly scope: Q01,Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/01-foundation` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Establish a reproducible native ESP-IDF repository foundation and identify the exact ESP32-C3 board and MD20A interface before assigning pins. Deliver a minimal safe startup skeleton, not operational pump control.

Document the exact board revision, flash, power path, RC input conditioning, RC/PWM pins, permanent MD20A DIR-to-GND jumper (no MCU DIR pin/control), and PWM boot/reset bias. Select and pin a stable ESP-IDF/toolchain and a portable C++ baseline suitable for a future constrained Nano adapter. Keep Wi-Fi and Bluetooth disabled. Provide real build and host-test entry points; do not present future platforms as buildable.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: A clean C3 build on documented dependencies; meaningful host-test harness check; inspection of disabled radio configuration; board/pin review and no-load startup/reset observations when hardware is available. A build does not prove physical boot safety.

Write `specs/01-foundation/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
