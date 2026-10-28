# Specify prompt — 05-failsafe-arming

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `05-failsafe-arming`. Requirement traceability: R07,R08,R09,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output. Questions to resolve or explicitly scope: Q04,Q05,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Complete the single-channel safety state machine and control integration before connecting the pump for functional operation.

Explicitly agree timeout and cutoff budget, valid-frame count, safe-low arming threshold, startup and fault recovery. Prevent reconnect-high from bypassing the selected safe recovery policy. Define handling of isolated invalid samples separately from stale input. Historical 100 ms is a proposal, not an approved requirement.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Timeout just before/at/after the chosen boundary; missing signal, stuck levels, invalid activity, high-command reconnect, safe-low recovery, startup, reset, deliberate task stall and watchdog; heavy diagnostics and old/overflow samples. Measure last-valid-to-LOW latency physically without pump before load acceptance.

Write `specs/05-failsafe-arming/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
