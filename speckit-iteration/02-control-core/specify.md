# Specify prompt — 02-control-core

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `02-control-core`. Requirement traceability: R03,R04,R05,R12. Prerequisites: 01-foundation. Questions to resolve or explicitly scope: Q04; Q05 remains unresolved until 05.
Set `SPECIFY_FEATURE_DIRECTORY=specs/02-control-core` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Specify a hardware-independent control core for one pump RC input, including validated measurements, a fixed linear map and explicit safe state boundaries.

Map valid nominal inputs 1000/1250/1400/1500/1800/2000 us to 0/25/40/50/80/100 percent. No endpoint learning. Validate before clamping. Agree width tolerance, rounding and immediate invalid-sample behavior; distinguish a proposed parameter from an accepted one. Define disarmed output as zero and keep production arming unavailable until 05.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: All six mapping points, endpoints and rounding boundaries; pulses outside agreed limits; invalid-to-high clamp regression; initial/disarmed zero; fault clearing not silently enabling output. Run on host without ESP32 headers.

Write `specs/02-control-core/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
