# Specify prompt — 08-esp32-portability

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `08-esp32-portability`. Requirement traceability: R02,R03,R08,R12,R14. Prerequisites: 07-ignition-channel. Questions to resolve or explicitly scope: Q08; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/08-esp32-portability` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

## Intended result

Deferred extension: support one explicitly selected additional ESP32 board without changing accepted core behavior.

Name the exact SoC, board and revision, supported channel count and electrical interface. Preserve mapping, arming, faults and diagnostic semantics. Recheck peripheral availability instead of assuming C3 and other ESP32 chips are identical.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Shared behavior fixtures and host suite; reproducible builds for C3 and the chosen target; physical timing/endpoints/capture/loss/recovery/boot/EMI acceptance for the new board. Compile success alone is not supported-hardware acceptance.

Write `specs/08-esp32-portability/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
