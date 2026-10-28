# Plan prompt — 08-esp32-portability

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `08-esp32-portability`. Requirement traceability: R02,R03,R08,R12,R14. Prerequisites: 07-ignition-channel. Questions to resolve or explicitly scope: Q08; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/08-esp32-portability` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

Read the accepted `specs/08-esp32-portability/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Deferred extension: support one explicitly selected additional ESP32 board without changing accepted core behavior.

Add a board profile and minimal platform-specific capture/output changes, reusing the portable core. Pin a compatible SDK/toolchain and verify RMT/LEDC availability, resource counts, pin conflicts, flash/USB/boot configuration and hardware fail-off. Keep existing C3 build functional; do not create placeholder targets for unspecified boards.

Preserve the accepted behavior: Name the exact SoC, board and revision, supported channel count and electrical interface. Preserve mapping, arming, faults and diagnostic semantics. Recheck peripheral availability instead of assuming C3 and other ESP32 chips are identical.

## Validation and deliverables

Shared behavior fixtures and host suite; reproducible builds for C3 and the chosen target; physical timing/endpoints/capture/loss/recovery/boot/EMI acceptance for the new board. Compile success alone is not supported-hardware acceptance.

Produce `specs/08-esp32-portability/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
