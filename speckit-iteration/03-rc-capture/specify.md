# Specify prompt — 03-rc-capture

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `03-rc-capture`. Requirement traceability: R05,R09,R11,R12. Prerequisites: 01-foundation,02-control-core. Questions to resolve or explicitly scope: Q03,Q04.
Set `SPECIFY_FEATURE_DIRECTORY=specs/03-rc-capture` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Capture and diagnose one RC pulse stream on the selected ESP32-C3 using RMT RX, with no load enabling.

Report pulse width, capture timestamp, observed period where available and explicit capture/validation faults. Handle short/long pulses, missing edges, stuck HIGH/LOW and malformed streams. Only accepted valid samples refresh last-valid time. Diagnostics are rate limited and are never another command source.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Synthetic measurement fixtures for malformed streams, timestamps and overflow; target build; logic-analyzer comparison with the RC tester and later receiver across agreed timing limits; capture under heavy serial diagnostics. Record measured vs expected timings.

Write `specs/03-rc-capture/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
