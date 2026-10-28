# Implementation Plan: [FEATURE]

**Iteration**: `[iteration]` | **Date**: [DATE] | **Spec**: [link]

**Input**: Feature specification from `specs/[iteration]/spec.md`

**Feature directory**: `SPECIFY_FEATURE_DIRECTORY=specs/[iteration]` (explicitly set)

**Note**: This template is filled in by the `/speckit-plan` command; its definition describes the execution workflow.

Write generated documentation and operator instructions in Ukrainian. Follow the adopted
constitution and roadmap; stop after the requested phase. Preserve input prompts and user
changes. Do not create/switch branches, commit, push, flash or operate loads automatically.

## Summary

[Extract from feature spec: primary requirement + technical approach from research]

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: [Standard C++ version selected under Q01; core has no ESP-IDF/GPIO dependencies]

**Primary Dependencies**: [Pinned native ESP-IDF, toolchain and host-test dependencies; Q01 decision]

**Storage**: [Only if required by the accepted scope; no learned endpoint calibration]

**Testing**: [Meaningful behavior tests; separate host, target-build and physical evidence]

**Target Platform**: [ESP32-C3 exact board/revision and approved profile under Q02 for v1]

**Project Type**: [Local embedded pump controller; portable core and platform adapters]

**Performance Goals**: [1000 Hz output; applicable capture/cutoff budgets with decision references]

**Constraints**: [Fixed 1000–2000 us mapping, radios disabled, nonblocking diagnostics;
startup/fail-off and hardware responsibilities relevant to this iteration]

**Scale/Scope**: [One pump input in v1; roadmap dependencies and explicit exclusions]

**Open Decisions**: [Applicable Q01–Q09, owner, due boundary and status. Resolve blocking
choices before affected implementation; later-owned questions do not block unrelated work.]

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Record pass, blocked or not applicable with a reason/reference for each gate, both before
research and after design. A blocked gate stops the affected work. Do not invent missing
parameters to pass a gate or use complexity justification as a waiver.

| Gate | Required check | Before research | After design |
|------|----------------|-----------------|--------------|
| I — Scope | One local pump channel, fixed mapping/1000 Hz, radios disabled; deferred features excluded. | [status/reference] | [status/reference] |
| II — Safety | Validity/freshness, startup, arming/recovery, cutoff and watchdog responsibilities; hardware bias and emergency cutoff separate; LOW means brake. | [status/reference] | [status/reference] |
| III — Architecture | Standard-C++ core, explicit platform adapters, approved profile boundaries, native ESP-IDF; no unsupported compatibility claims. | [status/reference] | [status/reference] |
| IV — Evidence | Pinned build/test dependencies or owned selection gate, behavior tests, traceability and separate host/build/physical evidence. | [status/reference] | [status/reference] |
| V — Workflow | Requested phase, explicit feature directory, roadmap dependencies, language policy and user changes preserved; no automatic Git/hardware actions. | [status/reference] | [status/reference] |
| VI — Decisions | Current sources and applicable Q owners/due boundaries; no silent timeout, threshold, pinout or suitability defaults. | [status/reference] | [status/reference] |

### Verification and Traceability

Map applicable R/FR IDs and acceptance scenarios to planned tasks and host tests,
target-build checks or physical checks. State tolerances and acceptance limits before
measurement. Keep actual evidence in `specs/[iteration]/verification/`; mark planned or
unperformed checks honestly. Test-first approval cycles are not required.

## Project Structure

### Documentation (this feature)

```text
specs/[iteration]/
├── plan.md              # This file (/speckit-plan command output)
├── research.md          # Phase 0 output (/speckit-plan command)
├── data-model.md        # Phase 1 output (/speckit-plan command)
├── quickstart.md        # Phase 1 output (/speckit-plan command)
├── contracts/           # Phase 1 output (/speckit-plan command)
├── tasks.md             # Separate /speckit-tasks output; NOT created by /speckit-plan
└── verification/        # Later actual host/build/physical evidence, kept separate
```

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this iteration. Retain only planned paths and distinguish existing components
  from proposed ones. A listed directory is not evidence of implementation.
-->

```text
# Planned boundaries; retain only paths relevant to this iteration.
firmware/
├── main/                       # Application composition
├── components/
│   ├── bridge_core/             # Portable standard C++
│   └── platform_esp32/          # ESP-IDF adapters
└── boards/                     # Explicit approved profiles
tests/                         # Host/adapter behavior tests
scripts/                       # Reproducible verification commands, if needed
```

**Structure Decision**: [Document the selected structure and reference the real
directories captured above]

## Complexity Tracking

> Record design tradeoffs only. Constitutional violations require correction or an
> explicitly adopted amendment; this table cannot waive a rule.

| Tradeoff | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [Design tradeoff, if any] | [Current iteration need] | [Evidence/rationale] |
