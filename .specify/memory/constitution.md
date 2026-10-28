<!--
Sync Impact Report
Version change: unratified upstream template (no approved version) -> 1.0.0.
Rationale: first adoption, not an amendment of the template's example principles.
Modified principles: none previously ratified; all six are newly adopted:
  I. Bounded Local Pump Control
  II. Explicit Safety and Fail-Off Responsibilities
  III. Portable Core and Verified Platform Boundaries
  IV. Reproducible Builds and Traceable Evidence
  V. Bounded Iterations and Authorized Actions
  VI. Honest Decisions and Scoped Uncertainty
Added sections: Operating Constraints; Development Workflow and Compliance; Governance.
Removed sections: unratified example principles and placeholder sections only.
Template synchronization:
  ✅ updated .specify/templates/plan-template.md: six-principle gates, firmware
     context, iteration paths, evidence and decision boundaries.
  ✅ updated .specify/templates/spec-template.md: scope, question ownership,
     requirement-to-acceptance traceability and distinct evidence categories.
  ✅ updated .specify/templates/tasks-template.md: required behavior verification,
     evidence tasks and action boundaries; removed mandatory test-first/commit defaults.
  ✅ reviewed .specify/templates/checklist-template.md: no change required.
  ✅ reviewed .specify/templates/constitution-template.md: upstream initialization
     template retained; it is not an approved project constitution.
Runtime guidance synchronization:
  ✅ updated README.md, AGENTS.md, speckit-iteration/README.md,
     docs/decisions.md and docs/roadmap.md to record adoption and workflow precedence.
Installed commands: reviewed all ten .agents/skills/speckit-*/SKILL.md files;
  no obsolete agent-specific execution references require edits. Upstream skills
  remain unchanged; generic defaults are subordinate to the project workflow below.
Extension hooks: .specify/extensions.yml absent; no pre/post hooks registered.
Deferred placeholders / manual synchronization TODOs: none.
Q01-Q09 remain owned by their roadmap iterations; adoption resolves none of them.
PROJECT_FOUNDATION.md and iteration input prompts are preserved unchanged.
-->
# rc-pwm-bridge Constitution

## Core Principles

### I. Bounded Local Pump Control

V1 MUST remain one local RC-PWM pump input on ESP32-C3, driving a Cytron MD20A with
fixed direction through native ESP-IDF. For valid nominal input, the mapping MUST
remain linear: 1000–2000 us maps to 0–100 percent duty. Output MUST use 1000 Hz at
intermediate duty, steady LOW at 0 percent and supported steady HIGH at 100 percent.
Automatic endpoint calibration MUST NOT be introduced. RC frame rate MUST NOT be
confused with output PWM frequency.

Wi-Fi and Bluetooth MUST remain disabled; firmware MUST NOT start network services or
require a network connection. Ignition ON/OFF, additional ESP32 boards and a concrete
Nano port MUST remain separate deferred iterations. This boundary keeps v1 acceptance
focused on agreed pump behavior rather than speculative extensions.

### II. Explicit Safety and Fail-Off Responsibilities

Control designs MUST expose startup/disarmed, armed, fault and recovery behavior,
signal validity and freshness, cutoff deadlines, and watchdog responsibilities.
The pump command MUST remain LOW before explicit arming and after a fault requires
cutoff. Arming and recovery conditions, including reconnecting a high command, MUST be
decided before the affected behavior is implemented. Missing or invalid input MUST
lead to fail-off under the selected policy and deadline; this constitution does not
select the single-invalid policy or timeout.

Validation MUST precede duty limiting. An invalid pulse MUST NOT become maximum output
through blind clamping, and invalid activity MUST NOT refresh last-valid time. Capture,
control and cutoff MUST remain nonblocking with respect to diagnostics; diagnostics
MUST be bounded and MUST NOT introduce another command source. Watchdog coverage,
triggering conditions and reset/recovery behavior MUST be documented and verified.

Safety analysis and acceptance MUST cover power application, reset, bootloader, upload,
brownout and watchdog recovery, including the interval before firmware executes.
Software shutdown, hardware control bias and independent emergency power cutoff MUST
be separate responsibilities. Before load acceptance, hardware design MUST define and
verify control bias and a physical emergency cutoff that removes load power independently
of MCU state and firmware. On MD20A, PWM LOW means brake; it MUST NOT be described
as coast, electrical isolation or proof of zero flow. A watchdog MUST NOT be treated as
independent power isolation. A low-side MOSFET's external flyback-diode arrangement
MUST NOT be copied onto the H-bridge without a verified driver-specific design. These
distinctions prevent a software command from being mistaken for removal of load power.

### III. Portable Core and Verified Platform Boundaries

The control core MUST use standard C++ and remain testable without MCU hardware.
GPIO, ESP-IDF and FreeRTOS dependencies MUST remain outside the core, including its
public headers. Capture, monotonic time and output access MUST cross explicit platform
adapter boundaries with documented units, validity and fault semantics.

Each supported board MUST have an explicit profile for its exact model/revision,
verified pins, polarity, peripheral resources and electrical assumptions. Native
ESP-IDF MUST be the ESP32 build environment; Arduino framework MUST NOT be added to v1.
A future Nano port MUST select a concrete Nano model, its own adapters, timer/resource
budget and separate build. It MUST NOT be represented as an ESP-IDF target.

Compatibility MUST NOT be claimed from a family name, illustrative pin table or build
alone. A new platform MUST preserve agreed core behavior unless an explicit decision
changes it. This separation supports reuse without promising unverified compatibility.

### IV. Reproducible Builds and Traceable Evidence

The owning iteration MUST select and pin SDK, toolchain and other build/test
dependencies, record relevant configuration and provide reproducible commands.
Q01 MUST be resolved before foundation implementation; adoption does not select
versions, the C++ standard or the host-test framework.

Changed control behavior MUST have meaningful tests of expected outcomes, boundaries
and applicable failure/recovery paths. Requirements MUST trace through acceptance
criteria and implementation tasks to verification evidence. Numerical tolerances,
cutoff budgets and physical acceptance limits MUST be defined before measurement.
Test-first development and separate approval of each test cycle are not mandated.

Host-test results, target-build evidence and physical acceptance MUST be recorded
separately, including commands/configuration, outcomes and unperformed checks.
Host tests MUST NOT be presented as proof of electrical behavior; a target build MUST
NOT be presented as proof of startup safety or physical cutoff. Measurements and
milestone completion MUST NOT be claimed without actual evidence. Documentation alone
completes neither firmware implementation nor a roadmap milestone.

### V. Bounded Iterations and Authorized Actions

Work MUST follow the dependencies and acceptance boundaries in `docs/roadmap.md`.
V1 comprises iterations 01–06; iterations 07–09 remain deferred until separately
requested. Scope or ordering changes MUST be recorded as explicit decisions.

Only the Spec Kit phase requested by the user MUST be invoked. Files under
`speckit-iteration/` MUST remain input prompts; generated feature artifacts MUST go
under `specs/<iteration>/`. Every feature-scoped invocation MUST explicitly set
`SPECIFY_FEATURE_DIRECTORY` to the intended iteration, independently of the Git branch
or stale active-feature state. Constitution adoption updates
`.specify/memory/constitution.md` and required synchronization files; it MUST NOT
create a feature specification or start another phase.

Branches MUST NOT be created/switched, commits or pushes made, firmware flashed, or
loads operated automatically. Each requires an appropriate explicit user task;
document generation or implementation alone does not authorize those actions.
Existing files and user changes MUST be preserved, with edits confined to authorized
scope. These rules keep automation reviewable and iteration boundaries real.

### VI. Honest Decisions and Scoped Uncertainty

Decisions, assumptions, proposals, manufacturer claims and measured results MUST be
distinguished. Blocking choices MUST be resolved and recorded in `docs/decisions.md`
and the owning iteration's specification before implementing the affected behavior.
Open questions MUST retain their owners and deadlines in `docs/open-questions.md`;
deferred questions MUST NOT block unrelated v1 work.

The proposed 100 ms timeout, 1400/1600 us ignition thresholds, shared two-channel
cutoff, illustrative pinout and electrical suitability MUST NOT become silent defaults.
Neither reported normal pump current nor a component rating establishes startup/stall
limits, thermal suitability or readiness to connect a load. A receiver can continue
producing valid pulses after radio loss; PWM freshness alone MUST NOT be claimed to
detect that event. Receiver failsafe behavior requires separate verification.

`PROJECT_FOUNDATION.md` MUST remain historical context and MUST NOT be rewritten to
imply current approval. Its two-channel start, WROOM preference and illustrative pins
are superseded by current decisions. Conflicting sources MUST be reconciled explicitly;
uncertainty MUST NOT be hidden to make an artifact appear complete.

## Operating Constraints

Read `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`,
`docs/roadmap.md` and `docs/open-questions.md` first; use `hardware/README.md` for local
document provenance and hardware caveats. Documentation and operator instructions MUST
be Ukrainian; input prompts and this constitution MUST be English. Upstream template
instructions may remain English; generated documentation MUST follow this policy.

Question ownership MUST remain as follows; adoption supplies no missing answer:

| Questions | Owning iteration and resolution boundary |
|-----------|-----------------------------------------|
| Q01 | 01: SDK/toolchain, C++ standard and host-test/build method before implementation. |
| Q02 | 01: exact C3 board, pins, resources, power and bias before approving its profile. |
| Q03 | 01/03: RC levels/interface before connection; source timing and loss behavior before capture acceptance. |
| Q04 | 02/03: validity, rounding/filtering, capture faults and tolerances before affected implementation. |
| Q05 | 05: timeout, cutoff budget, arming/recovery, watchdog and diagnostics before operational output. |
| Q06 | 01 hardware basis, then 06 refinement/acceptance: power path, protection, bias, emergency cutoff and load limits. |
| Q07 | 07: ignition interface, thresholds, interlocks and cross-channel faults; does not block 01–06. |
| Q08 | 08: concrete additional ESP32 board/peripherals; does not block v1. |
| Q09 | 09: concrete Nano, toolchain, timers, levels and resource budget; does not block v1. |

Before iteration 05 is implemented and accepted, output MUST remain limited to isolated
verification without the pump. Iterations 03–05 MUST be verified without an operational
pump. Controlled pump validation in 06 MUST depend on accepted 05 behavior and a verified
power bench. Unresolved Q02–Q06 dependencies MUST prevent claims of hardware suitability
at the affected boundary, without freezing unrelated portable-core work.

## Development Workflow and Compliance

Every specification MUST identify its roadmap scope and exclusions, applicable
requirement IDs, acceptance scenarios, evidence categories and owned/open decisions.
Every plan MUST contain a Constitution Check before research and repeat it after design.
Each of these six gates MUST record pass, blocked or not applicable, with a reason and
reference. A later-owned unresolved question is not itself a failure for unrelated work,
but MUST be linked to its owner and resolution boundary.

1. Scope: one local pump channel and fixed mapping/frequency for v1; deferred work excluded.
2. Safety: validity/freshness, startup, arming/recovery, cutoff, watchdog and hardware
   responsibilities covered for affected behavior, without invented parameters.
3. Architecture: portable core, adapters and verified board-profile boundaries;
   no unsupported compatibility claim.
4. Evidence: pinned dependencies or an owned selection gate, meaningful behavior tests,
   requirement-to-acceptance traceability and separate host/build/physical records.
5. Workflow: authorized phase, explicit feature directory, roadmap dependencies,
   language policy, user changes preserved and no automatic Git/hardware actions.
6. Decisions: current sources used, blocking choices resolved at their due boundary,
   and deferred questions retained without obstructing unrelated scope.

A blocked gate MUST prevent the affected design/implementation from proceeding until
resolved. A complexity justification alone MUST NOT waive a constitutional rule.
Reviews of requirements, plans, tasks and changes MUST check applicable principles and
traceability. Tasks MUST include verification and evidence recording alongside affected
behavior; physical checks that cannot be performed MUST remain open. Iteration acceptance
MUST compare evidence with predeclared criteria and record failures, missing equipment
and unperformed checks explicitly.

Installed Spec Kit skills provide workflow mechanics. Their generic defaults about
optional tests, test-first ordering, guessed choices, clarification limits or automatic
actions MUST NOT override these project rules. A clarification limit MUST NOT force an
unresolved blocking choice into a default; record remaining decisions and request their
resolution only when due. No phase suggestion or extension hook authorizes another phase
or a Git/hardware action beyond the user's task.

## Governance

This is the first approved constitution, adopted by the user's explicit constitution
task on 2026-10-28. The preceding file was an unratified upstream template and carried no
approved principles or version. Adoption establishes governance only; firmware remains
unimplemented and physical acceptance has not been performed.

This constitution governs specifications, plans, tasks, implementation and acceptance.
Current explicit user instructions take precedence; a conflicting instruction MUST be
recorded and reconciled through an explicit amendment rather than silently leaving
contradictory guidance. Historical proposals and upstream examples MUST NOT override
ratified principles or current decisions.

An amendment MUST describe the rule change, rationale, affected requirements/artifacts,
and any migration or re-verification required. The project owner/user MUST explicitly
authorize adoption of changed governance or principles; an explicit constitution-update
task provides that authorization within its stated scope. The update MUST synchronize
affected templates and runtime guidance, prepend a Sync Impact Report, and record
remaining follow-up items. No commit is required for adoption.

Versions MUST follow MAJOR.MINOR.PATCH: MAJOR for incompatible removal or redefinition
of a principle/governance obligation; MINOR for new principles/sections or materially
expanded obligations that preserve existing ones; PATCH for clarifications or editorial
corrections without changed obligations. Ratified MUST retain the original adoption
date; Last Amended MUST record the latest adopted change date. Dates MUST use YYYY-MM-DD.
The version footer and Sync Impact Report MUST agree.

**Version**: 1.0.0 | **Ratified**: 2026-10-28 | **Last Amended**: 2026-10-28
