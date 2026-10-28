# Specify prompt — 04-pump-output

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `04-pump-output`. Requirement traceability: R01,R04,R06,R08,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture. Questions to resolve or explicitly scope: Q02,Q04,Q06; Q05 must not be bypassed.
Set `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Generate and measure the MD20A control waveform with an isolated no-pump test setup before operational arming exists.

At intermediate duty generate 1000 Hz PWM. Zero is continuous LOW and full duty is the supported continuous HIGH, with fixed direction. Startup, disarmed and faults select LOW. MD20A LOW means brake, not coast or power isolation. Do not enable automatic RC-to-pump operation before 05.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Build plus adapter/core tests where meaningful; measure frequency and all six nominal duty points with a logic analyzer; verify static endpoints, direction and boot/reset/upload traces. Document physical tolerances before testing and distinguish disconnected-driver vs powered-driver observations.

Write `specs/04-pump-output/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
