# Plan prompt — 02-control-core

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `02-control-core`. Requirement traceability: R03,R04,R05,R12. Prerequisites: 01-foundation. Questions to resolve or explicitly scope: Q04; Q05 remains unresolved until 05.
Set `SPECIFY_FEATURE_DIRECTORY=specs/02-control-core` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/02-control-core/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Specify a hardware-independent control core for one pump RC input, including validated measurements, a fixed linear map and explicit safe state boundaries.

Use a small standard-C++ component without ESP-IDF/FreeRTOS includes, dynamic platform dependencies or I/O. Define measurement, validation result, normalized duty, state/fault and explicit-time update interfaces with units and ranges. Use integer-safe arithmetic and a fake monotonic clock. Defer exact timeout/rearm policy to 05 rather than hardcoding the historical 100 ms proposal.

Preserve the accepted behavior: Map valid nominal inputs 1000/1250/1400/1500/1800/2000 us to 0/25/40/50/80/100 percent. No endpoint learning. Validate before clamping. Agree width tolerance, rounding and immediate invalid-sample behavior; distinguish a proposed parameter from an accepted one. Define disarmed output as zero and keep production arming unavailable until 05.

## Validation and deliverables

All six mapping points, endpoints and rounding boundaries; pulses outside agreed limits; invalid-to-high clamp regression; initial/disarmed zero; fault clearing not silently enabling output. Run on host without ESP32 headers.

Produce `specs/02-control-core/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
