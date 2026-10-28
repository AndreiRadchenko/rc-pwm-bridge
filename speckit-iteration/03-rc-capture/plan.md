# Plan prompt — 03-rc-capture

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `03-rc-capture`. Requirement traceability: R05,R09,R11,R12. Prerequisites: 01-foundation,02-control-core. Questions to resolve or explicitly scope: Q03,Q04.
Set `SPECIFY_FEATURE_DIRECTORY=specs/03-rc-capture` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/03-rc-capture/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Capture and diagnose one RC pulse stream on the selected ESP32-C3 using RMT RX, with no load enabling.

Use the modern RMT RX API of the pinned SDK. Choose and justify resolution, duration limits, glitch filter, buffer size and rearm sequence against actual tester/receiver timing. Bound ISR work, safely hand snapshots to the control task, detect overflow and dropped events. Avoid losing a valid pulse at RX rearm or treating malformed activity as freshness. Keep output LOW.

Preserve the accepted behavior: Report pulse width, capture timestamp, observed period where available and explicit capture/validation faults. Handle short/long pulses, missing edges, stuck HIGH/LOW and malformed streams. Only accepted valid samples refresh last-valid time. Diagnostics are rate limited and are never another command source.

## Validation and deliverables

Synthetic measurement fixtures for malformed streams, timestamps and overflow; target build; logic-analyzer comparison with the RC tester and later receiver across agreed timing limits; capture under heavy serial diagnostics. Record measured vs expected timings.

Produce `specs/03-rc-capture/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
