# Feature Specification: [FEATURE NAME]

**Iteration**: `[iteration]`

**Feature directory**: `SPECIFY_FEATURE_DIRECTORY=specs/[iteration]` (explicitly set)

**Created**: [DATE]

**Status**: Draft

**Input**: User description: "$ARGUMENTS"

Write the generated specification in Ukrainian. Follow `.specify/memory/constitution.md`;
retain input prompts in `speckit-iteration/` and preserve user changes. Invoke only the
requested phase; do not create/switch branches, commit, push, flash or operate loads
automatically.

## Scope and Decision Boundaries *(mandatory)*

- **Roadmap scope and dependencies**: [Owning iteration, prerequisites and applicable R IDs]
- **Included behavior**: [Bounded contribution to one local pump channel for v1]
- **Excluded/deferred work**: [Ignition, other ESP32 boards and Nano belong to 07–09]
- **Agreed invariants**: [Fixed 1000–2000 us → 0–100%, 1000 Hz output, no automatic
  calibration, radios disabled; refer to existing constraints without inventing parameters]
- **Decisions**: [Applicable Q01–Q09, owner, status and resolution boundary; later-owned
  questions do not block unrelated work. No silent 100 ms timeout, ignition thresholds,
  cross-channel cutoff, pinout or electrical suitability defaults.]

## User Scenarios & Testing *(mandatory)*

<!--
  IMPORTANT: User stories should be PRIORITIZED as user journeys ordered by importance.
  Each story must be independently verifiable within its declared dependencies.
  A partial increment does not authorize pump operation or bypass roadmap safety gates.

  Assign priorities (P1, P2, P3, etc.) to each story, where P1 is the most critical.
  Think of each story as a standalone slice of functionality that can be:
  - Developed independently
  - Tested independently
  - Deployed independently
  - Demonstrated to users independently
-->

### User Story 1 - [Brief Title] (Priority: P1)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently - e.g., "Can be fully tested by [specific action] and delivers [specific value]"]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]
2. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

### User Story 2 - [Brief Title] (Priority: P2)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

### User Story 3 - [Brief Title] (Priority: P3)

[Describe this user journey in plain language]

**Why this priority**: [Explain the value and why it has this priority level]

**Independent Test**: [Describe how this can be tested independently]

**Acceptance Scenarios**:

1. **Given** [initial state], **When** [action], **Then** [expected outcome]

---

[Add more user stories as needed, each with an assigned priority]

### Edge Cases

<!--
  ACTION REQUIRED: The content in this section represents placeholders.
  Fill them out with the right edge cases.
-->

- [Applicable mapping/validity boundaries; invalid input must not clamp to maximum duty]
- [Missing/stale/malformed capture and last-valid timestamp behavior]
- [Startup/reset, arming, reconnect-high, watchdog and diagnostics under the owned scope]
- [Physical responsibilities and evidence boundaries; MD20A LOW means brake, not isolation]

## Requirements *(mandatory)*

<!--
  ACTION REQUIRED: The content in this section represents placeholders.
  Fill them out with the right functional requirements.
-->

### Functional Requirements

- **FR-001**: System MUST [testable iteration behavior, traced to applicable R ID]
- **FR-002**: System MUST [applicable validity/freshness or fail-off behavior]
- **FR-003**: System MUST [applicable startup/arming/recovery behavior]

*Example of marking unclear requirements:*

- **FR-004**: System MUST [affected behavior] under
  [NEEDS CLARIFICATION: question ID, owner and decision needed before implementation].

### Key Entities *(include if feature involves data)*

- **[Entity 1]**: [What it represents, key attributes without implementation]
- **[Entity 2]**: [What it represents, relationships to other entities]

## Success Criteria *(mandatory)*

<!--
  ACTION REQUIRED: Define measurable success criteria.
  These must describe measurable behavior and respect agreed hardware/platform constraints.
  Define numerical tolerances before measurement; do not invent values for open decisions.
-->

### Measurable Outcomes

- **SC-001**: [Measurable outcome and tolerance for the iteration's accepted behavior]
- **SC-002**: [Applicable fault/recovery outcome within an explicitly selected deadline]

### Acceptance Traceability and Evidence *(mandatory)*

| Requirement R/FR ID | Scenario / SC ID | Verification method | Evidence location / status |
|-------------------|------------------|---------------------|----------------------------|
| [ID] | [ID] | [Host test, target build or physical check] | [Planned path; not performed] |

Keep host tests, target-build evidence and physical acceptance separate. Record actual
results later under `specs/[iteration]/verification/`; a build does not prove boot safety
or physical cutoff. Unperformed checks and unaccepted milestones remain open.

## Assumptions

<!--
  ACTION REQUIRED: The content in this section represents placeholders.
  Distinguish assumptions from agreed decisions and measurements. Resolve blocking choices
  before implementing affected behavior. Do not force Q01–Q09 into defaults to meet a
  clarification limit; record deferred questions with their owner and due boundary.
-->

- [Assumption, source, confidence and how/when it will be verified]
- [Dependency on an approved profile, selected toolchain or prior iteration acceptance]
