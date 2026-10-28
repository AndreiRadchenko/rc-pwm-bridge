# Plan prompt — 05-failsafe-arming

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `05-failsafe-arming`. Requirement traceability: R07,R08,R09,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output. Questions to resolve or explicitly scope: Q04,Q05,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/05-failsafe-arming/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Complete the single-channel safety state machine and control integration before connecting the pump for functional operation.

Integrate capture, monotonic time, validation, mapping and output in a nonblocking task with fail-off priority. Choose watchdog supervision and reset behavior; a watchdog reset is not independent load isolation. Specify queue overflow, task stall and diagnostic congestion behavior and ensure old samples cannot re-arm. Include time wrap/long uptime handling appropriate to the chosen time type.

Preserve the accepted behavior: Explicitly agree timeout and cutoff budget, valid-frame count, safe-low arming threshold, startup and fault recovery. Prevent reconnect-high from bypassing the selected safe recovery policy. Define handling of isolated invalid samples separately from stale input. Historical 100 ms is a proposal, not an approved requirement.

## Validation and deliverables

Timeout just before/at/after the chosen boundary; missing signal, stuck levels, invalid activity, high-command reconnect, safe-low recovery, startup, reset, deliberate task stall and watchdog; heavy diagnostics and old/overflow samples. Measure last-valid-to-LOW latency physically without pump before load acceptance.

Produce `specs/05-failsafe-arming/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
