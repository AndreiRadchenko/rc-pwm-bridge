# Plan prompt — 07-ignition-channel

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `07-ignition-channel`. Requirement traceability: R03,R08,R09,R12,R13. Prerequisites: 06-pump-validation. Questions to resolve or explicitly scope: Q07; revisit Q02,Q04,Q05,Q06 for two channels.
Set `SPECIFY_FEATURE_DIRECTORY=specs/07-ignition-channel` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches during Plan, commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

Read the accepted `specs/07-ignition-channel/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Deferred extension: add a second independent RC input for ignition/glow-plug ON/OFF, preserving the accepted pump channel.

Extend the core with an ignition state and independent validity/timestamp, a second capture adapter resource and an ignition-output abstraction. Audit C3 RMT channel allocation and pin availability. Define safety overrides explicitly, including whether either input fault disables both outputs. Reuse pump mapping unchanged and document any approved compatibility change.

Preserve the accepted behavior: Identify whether ignition is direct plug power, ignition-unit supply or a control input before selecting its output. Agree thresholds/hysteresis, power/interface ratings and isolation needs, per-channel freshness and cross-channel fault/recovery rules. Startup in the hysteresis band stays OFF; a fault must not retain ON. Historical 1400/1600 us and shared cutoff are proposals only. Do not introduce automatic ignition sequencing.

Read the accepted 04–06 specs/contracts and acceptance evidence at invocation time. Preserve pump width/period validation, estimated-edge freshness, mapping/rounding, diagnostic fault history, accepted D64/D65 operational Test/Fly and automatic recovery policy, and bounded diagnostics. Do not turn historical pump signal-loss latches into permanent output inhibition or copy ignition OFF-on-fault into Fly pump behavior. Use one monotonic domain for both channels; extend independent status/counters without turning diagnostics into per-edge history. Check simultaneous capture/timing/loss under maximum diagnostics; add pins/resources only through the updated exact-board profile. Do not assign ignition the pump's validity/arming thresholds by assumption. Pump DIR remains a hardware GND jumper.

## Validation and deliverables

Regression suite for pump; ignition boundaries and startup band; both inputs active, either/both lost, independent freshness and chosen cross-channel policy; boot/reset/recovery of both physical commands. Separate interface-only tests from actual ignition-load acceptance.

Produce `specs/07-ignition-channel/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
